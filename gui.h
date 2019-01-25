
struct Button
{
   Rectangle2D rect;
   b32 active;
};

struct TextInput
{
	char inputBuffer[256];
   u32 amount;
};

static b32 MaybeAddChar(TextInput *t, Char c)
{
   //..
}

struct ValueDisplayEntry
{
   u32 type;
   // these are in the entry buffer
   String values[10];
   u32 valueAmount;
};

enum ValueDisplayState 
{
   ValueDisplay_None,
   ValueDisplay_DraggingDisplay,
   ValueDisplay_Scrolling,
   ValueDisplay_Resizing,
};

static ValueDisplayTransientInfo
{
   ValueDisplayState state; // this can be in the gui struct, i.e the gui just has a few states.
   f32 scrollOffset; // we caould have memory for this in gui, dependent on __File__, __line__
   Rectangle2D rect; 
   // this memory should get cleared on the end of a frame,  so we could have like a double buffer.
   // plus we need a count, for loops.
};

struct ValueDisplay
{
   GUI_ElementType type;
   
   union
   {
      ValueDisplayTransientInfo info;
      struct
      {
         ValueDisplayState state;
         f32 scrollOffset;
         Rectangle2D rect;
      };
   };
   u32 amountOfEntries = 0;
   ValueDisplayEntry entries[30];
   
   u8  entryBuffer[512];
   u32 entryBufferAt = 0;
   
   ValueDisplayEntry *currentEntry;
};



enum GUI_ElementType
{
   GUI_NONE,
   
   GUI_VALUE_DISPLAY,
   
   GUI_COUNT
};

struct GUI_Element
{
   GUI_ElementType type;
   u32 parent;
};

struct GUI
{
   struct  // theme
   {
      v4 borderColor     = V4(1.0f, 0.2f, 0.2f, 0.2f);
      v4 backGroundColor = V4(1.0f, 0.5f, 0.7f, 0.9f);
      v4 entryColor      = V4(1.0f, 0.2f, 0.4f, 0.8f);
      v4 headerColor     = V4(1.0f, 0.2f, 0.4f, 0.8f);
      
      f32 borderSize     = 0.006f;
      f32 headerSize     = 0.1f;
      f32 fontSize       = 0.05f;
   };
   
   TextInput textInput;
   
   
   u8 buffer[2][2048];
   
   u32 bufferUsed;
   
   GUI_Element *elements[40];
   u32 amountOfElements = 0;
};



static ValueDisplay *PushValueDisplay(GUI *gui, ValueDisplayTransientInfo info)
{
   if(gui->amountOfElements >= ArrayCount(gui->elements))
   {
      Die;
      return;
   }
   
   ValueDisplay *valueDisplay = PushStruct(frameArena, ValueDisplay);
   valueDisplay->info = info;
   
   return valueDisplay;
}

static void Reset(ValueDisplay *display)
{
   display->amountOfEntries = 0;
   display->entryBufferAt = 0;
}

// we assume you alloced  string  somehow, frameArena or sth
static void AddEntry(ValueDisplay *display, u32 type)
{
   if(display->amountOfEntries + 1 < ArrayCount(display->entries))
   {
      ValueDisplayEntry *entry = display->entries + display->amountOfEntries++;
      entry->type = type;
      entry->valueAmount = 0;
      display->currentEntry = entry;
   }
}

static void AddValue(ValueDisplay *display, char *format, ...)
{
   if(display->currentEntry->valueAmount == ArrayCount(display->currentEntry->values))
   {
      Die;
      return;
   }
   
   Arena arena = CreateArena(display->entryBuffer + display->entryBufferAt, ArrayCount(display->entryBuffer) - display->entryBufferAt);
   
   va_list argList;
   va_start(argList, format);
   String toPrint = StringFormatHelper(&arena, format, argList);
   va_end(argList);
   
   // todo this is a bit of a hack, what we really want here is just a buffer.
   display->entryBufferAt += (u32)(arena.current - arena.base);
   
   display->currentEntry->values[display->currentEntry->valueAmount++] = toPrint;
}

static void PushRectangle(RenderGroup *rg, Rectangle2D rect, u32 layer, v4 color)
{
   PushRectangle(rg, rect.pos, rect.width, rect.height, layer, color);
}
static void PushRelRect(RenderGroup *rg, Rectangle2D fit, v2 offset, f32 width, f32 height, i32 layer, v4 color)
{
   PushRectangle(rg, fit.pos + offset * fit.dimensions, fit.width * width, fit.height * height, layer, color);
}

static void PushRelString(RenderGroup *rg, Rectangle2D fit, char *string, v2 offset, f32 size, i32 layer)
{
   PushString(rg, fit.pos + offset * fit.dimensions, layer, string, size * fit.height);
}

static void PushRelString(RenderGroup *rg, Rectangle2D fit, char *string, u32 strlen, v2 offset, f32 size, i32 layer)
{
   PushString(rg, fit.pos + offset * fit.dimensions, layer, (Char *)string, strlen, size * fit.height);
}

static void RenderScrollbar(RenderGroup *rg, Rectangle2D dim, i32 baseLayer, f32 border, f32 totalEntrySize, f32 scrollOffset, v4 backColor, v4 barColor)
{
   i32 backLayer = baseLayer + 0;
   i32 barLayer  = baseLayer - 1;
   
   PushRectangle(rg, dim, backLayer, backColor);
   
   f32 fieldHeight = 1.0f;
   
   dim.pos += V2(border, border) * dim.dimensions;
   dim.dimensions -= 2.0f * border * dim.dimensions;
   
   if(totalEntrySize > fieldHeight) 
   {
      Assert(scrollOffset < totalEntrySize);
      f32 scrollbarHeight = fieldHeight / totalEntrySize;
      f32 halfHeight      = scrollbarHeight / 2.0f;
      
      f32 scrollbarPos    = scrollOffset / totalEntrySize; 
      scrollbarPos        = MapRangeToRangeCapped(scrollbarPos, 0.0f, 1.0f, halfHeight, 1.0f - halfHeight);
      
      PushRelRect(rg, dim, V2(0, scrollbarPos - halfHeight), 1.0f, scrollbarHeight , barLayer, barColor);
   }
}

static Rectangle2D FitRectangle(Rectangle2D fit, v2 p, f32 width, f32 height)
{
   Rectangle2D ret;
   ret.pos    = fit.pos + fit.dimensions * p;
   ret.width  = fit.width * width;
   ret.height = fit.height * height;
   return ret;
}

static void ValueDisplayHandleEvents(ValueDisplay *display, Input input, KeyStateMessage message)
{
   if(!display->active) return;
   
   if(message.flags & KeyState_PressedThisFrame)
   {
      switch(message.key)
      {
         case Key_leftMouse:
         {
            v2 p = input.mouseZeroToOne;
            if(!PointInRectangle(display->rect, p)) return;
            
            if(PointInRectangle(display->rect.pos, , p))
            {
               display->state = ValueDisplay_DraggingDisplay;
               return;
            }
            
            if(PointInRectangle(display->))
            {
               
            }
            
         }break;
      }
   }
   
   if(message.flags & KeyState_ReleasedThisFrame)
   {
      
   }
   
}


static void RenderValueDisplay(RenderGroup *rg, ValueDisplay *display, u32 baseLayer)
{
   if(!display->active) return;
   
   i32 borderLayer    = baseLayer - 0;
   i32 backLayer      = baseLayer - 1;
   //i32 entryBackLayer = baseLayer - 2;
   i32 entryLayer     = baseLayer - 3;
   i32 stringLayer    = baseLayer - 4;
   
   f32 absoluteBorder = display->borderSize;
   
   Rectangle2D withBorder;
   withBorder.pos    = display->rect.pos - V2(absoluteBorder, absoluteBorder);
   withBorder.width  = display->rect.width  + 2.0f * absoluteBorder;
   withBorder.height = display->rect.height + 2.0f * absoluteBorder;
   
   PushRectangle(rg, withBorder, borderLayer, display->borderColor);
   PushRectangle(rg, display->rect, backLayer, display->backGroundColor);
   
   // render variables
   Rectangle2D fit;
   fit.pos    = display->rect.pos + V2(absoluteBorder, absoluteBorder);
   fit.width  = display->rect.width  - 2.0f * absoluteBorder;
   fit.height = display->rect.height - 2.0f * absoluteBorder;
   
   if(fit.width <= 0.0f || fit.height <= 0.0f) return;
   
   f32 hBorder = absoluteBorder / fit.height;
   
   // header
   PushRelRect(rg, fit, V2(), 1.0f, display->headerSize, entryLayer, display->headerColor);
   PushRelString(rg, fit, (char *)display->headerString.data, display->headerString.amount, V2(), display->fontSize, stringLayer);
   
   Rectangle2D entryRect = FitRectangle(fit, V2(0, (display->headerSize + hBorder)), 1.0f, 1.0f - (display->headerSize + hBorder));
   
   f32 relFontSize = display->fontSize;
   f32 totalHeight = 0.0f;
   Each(i, display->amountOfEntries)
   {
      ValueDisplayEntry *entry = display->entries + i;
      totalHeight += (f32)entry->valueAmount * relFontSize + hBorder;
   }
   
   {   // scrollbar
      Rectangle2D scrollbarRect = FitRectangle(entryRect, V2(1.0f - display->scrollbarWidth, 0.0f), display->scrollbarWidth, 1.0f);
      
      f32 scrollOffset = display->scrollOffset;
      RenderScrollbar(rg, scrollbarRect, stringLayer, hBorder, totalHeight, scrollOffset, display->entryColor, display->backGroundColor);
   }
   
   {   // entries
      v2 p = V2();
      Each(i, display->amountOfEntries)
      {
         ValueDisplayEntry *entry = display->entries + i;
         u32 bufferAt = 0;
         f32 thisHeight = (f32)entry->valueAmount * relFontSize;
         
         Each(j, entry->valueAmount)
         {
            String *it = entry->values + j;
            PushRelString(rg, entryRect, (char *)it->data, it->length, p + V2(0.0f, j * relFontSize), relFontSize, stringLayer);
         }
         
         PushRelRect(rg, entryRect, p, 1.0f, thisHeight, entryLayer, display->entryColor);
         p.y += thisHeight + hBorder;
      }
   }
   
}