
// todo rename to GUIUtil.h

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

static void PushRectangle(RenderGroup *rg, Rectangle2D rect, u32 layer, v4 color)
{
   PushRectangle(rg, rect.pos, rect.width, rect.height, layer, color);
}
static void PushRelRect(RenderGroup *rg, Rectangle2D fit, v2 offset, f32 width, f32 height, i32 layer, v4 color)
{
   PushRectangle(rg, fit.pos + offset * fit.dimensions, fit.width * width, fit.height * height, layer, color);
}
static void PushUnscaledRect(RenderGroup *rg, Rectangle2D fit, v2 offset, f32 width, f32 height, i32 layer, v4 color)
{
   PushRectangle(rg, fit.pos + offset * fit.dimensions,  width,  height, layer, color);
}


static void PushRelString(RenderGroup *rg, Rectangle2D fit, char *string, v2 offset, f32 size, i32 layer)
{
   PushString(rg, fit.pos + offset * fit.dimensions, layer, string, size * fit.height);
}

static void PushRelString(RenderGroup *rg, Rectangle2D fit, char *string, u32 strlen, v2 offset, f32 size, i32 layer)
{
   PushString(rg, fit.pos + offset * fit.dimensions, layer, (Char *)string, strlen, size);
}

static Rectangle2D FitRectangle(Rectangle2D fit, v2 p, f32 width, f32 height)
{
   Rectangle2D ret;
   ret.pos    = fit.pos + fit.dimensions * p;
   ret.width  = fit.width * width;
   ret.height = fit.height * height;
   return ret;
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
