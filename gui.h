
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
   StringArray values; // we assume you alloced this somehow, frameArena or sth
};

enum ValueDisplayState 
{
   ValueDisplay_None,
   ValueDisplay_DragingDisplay,
   
};

struct ValueDisplay
{
   ValueDisplayState state;
   
   String headerString;
   u32 amountOfEntries;
   ValueDisplayEntry entries[30];
   
   Rectangle2D rect;
   u32 hotEntry; // unactive if hotValue == 0xFFFFFFFF;
   u32 hotValue;
   b32 visible;
   
   v4 borderColor;
   v4 backGroundColor;
   v4 entryColor;
   v4 headerColor;
   
   f32 borderSize;
   f32 headerSize;
   f32 fontSize;
};

static void Reset(ValueDisplay *display)
{
   display->amountOfEntries = 0;
}

// we assume you alloced  string  somehow, frameArena or sth
static void AddEntry(ValueDisplay *display, u32 type, StringArray values)
{
   if(display->amountOfEntries + 1 < ArrayCount(display->entries))
   {
      ValueDisplayEntry *entry = display->entries + display->amountOfEntries++;
      entry->type = type;
      entry->values = values;
   }
}

static void PushRectangle(RenderGroup *rg, Rectangle2D rect, u32 layer, v4 color)
{
   PushRectangle(rg, rect.pos, rect.width, rect.height, layer, color);
}
static void PushRelRect(RenderGroup *rg, Rectangle2D fit, v2 offset, f32 width, f32 height, u32 layer, v4 color)
{
   PushRectangle(rg, fit.pos + offset, fit.width * width, fit.height * height, layer, color);
}

static void PushRelString(RenderGroup *rg, Rectangle2D fit, char *string, v2 offset, f32 size, u32 layer)
{
   PushString(rg, fit.pos + offset, layer, string, size * fit.height);
}

static void PushRelString(RenderGroup *rg, Rectangle2D fit, char *string, u32 strlen, v2 offset, f32 size, u32 layer)
{
   PushString(rg, fit.pos + offset, layer, (Char *)string, strlen, size * fit.height);
}

static void RenderValueDisplay(RenderGroup *rg, ValueDisplay *display, u32 baseLayer)
{
   if(!display->visible) return;
   
   i32 borderLayer = baseLayer + 1;
   i32 backLayer   = baseLayer - 0;
   i32 entryLayer  = baseLayer - 1;
   i32 stringLayer = baseLayer - 2;
   
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
   v2 p = V2(0, 0);
   
   PushRelRect(rg, fit, p, 1.0f, display->headerSize, entryLayer, display->headerColor);
   PushRelString(rg, fit, (char *)display->headerString.data, display->headerString.amount, p, display->fontSize, stringLayer);
   p.y += display->headerSize + hBorder;
   
   Each(i, display->amountOfEntries)
   {
      ValueDisplayEntry *entry = display->entries + i;
      f32 height = 0.0f;
      u32 bufferAt = 0;
      
      For(entry->values)
      {
         PushRelString(rg, fit, (char *)it->data, it->length, p + V2(0.0f, height), display->fontSize, stringLayer);
         height += display->fontSize;
      }
      
      PushRelRect(rg, fit, p, 1.0f, height, entryLayer, display->entryColor);
      p.y += height;
   }
}