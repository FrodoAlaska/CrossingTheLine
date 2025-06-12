#include "ui.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// UILayout functions

void ui_layout_create(UILayout* layout, 
                      nikola::Window* window_ref, 
                      const nikola::ResourceID& font_id, 
                      const OnUILayoutClickFunc& click_func, 
                      const void* user_data) {
  NIKOLA_ASSERT(layout, "Invalid UILayout given to ui_layout_create");
  NIKOLA_ASSERT(RESOURCE_IS_VALID(font_id), "Invalid font given to ui_layout_create");

  // Variables init

  layout->font_id    = font_id; 
  layout->window_ref = window_ref;

  layout->click_func = click_func;
  layout->user_data  = (void*)user_data; 
}

void ui_layout_begin(UILayout& layout, const UIAnchor anchor, const nikola::Vec2& offset) {
  layout.current_anchor = anchor; 
  layout.extra_offset   = offset;
}

void ui_layout_end(UILayout& layout) {
  layout.extra_offset   = nikola::Vec2(0.0f);
  layout.current_offset = nikola::Vec2(0.0f);
}

void ui_layout_push_text(UILayout& layout, const nikola::String& str, const float size, const nikola::Vec4& color) {
  // Create the text

  layout.texts.push_back(UIText{});
  UIText* text = &layout.texts[layout.texts.size() - 1];

  UITextDesc text_desc = {
    .string = str, 

    .font_id   = layout.font_id, 
    .font_size = size, 

    .anchor = layout.current_anchor, 
    .offset = layout.current_offset,

    .color = color,
  };
  ui_text_create(text, layout.window_ref, text_desc);

  // Apply the offset for the next UI element
  layout.current_offset += layout.extra_offset;
}

void ui_layout_update(UILayout& layout) {
  if(nikola::input_key_pressed(nikola::KEY_UP)) {
    layout.current_option--;
  }
  else if(nikola::input_key_pressed(nikola::KEY_DOWN)) {
    layout.current_option++;
  }

  // Wraping around to avoid seg faults
  if(layout.current_option < 0) {
    layout.current_option = layout.texts.size() - 1;
  } 
  else if(layout.current_option >= layout.texts.size()) {
    layout.current_option = 0;
  } 
 
  // "Applying" the option and invoking the callback if it exists
  if(nikola::input_key_pressed(nikola::KEY_ENTER) && layout.click_func) {
    layout.click_func(layout, layout.texts[layout.current_option], layout.user_data);
  }
}

void ui_layout_render(UILayout& layout) {
  // Render the texts
  for(auto& text : layout.texts) {
    ui_text_render(text);
  }

  // Render the cursor
  
  UIText* current_text  = &layout.texts[layout.current_option];
  nikola::batch_render_text(current_text->font, 
                            ">",
                            current_text->position - nikola::Vec2(current_text->font_size, 0.0f),
                            current_text->font_size, 
                            current_text->color); 
}

/// UILayout functions
/// ----------------------------------------------------------------------
