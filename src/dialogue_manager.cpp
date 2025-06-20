#include "dialogue_manager.h"

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// DialogueManager
struct DialogueManager {
  nikola::DynamicArray<nikola::String> lines;
  nikola::sizei current_line = 0;
};

static DialogueManager s_manager;
/// DialogueManager
/// ----------------------------------------------------------------------

/// ----------------------------------------------------------------------
/// Dialogue manager functions

void dialogue_manager_init(const nikola::FilePath& txt_path) {
  // Open the file
  nikola::File file; 
  if(!nikola::file_open(&file, txt_path, (int)(nikola::FILE_OPEN_READ))) {
    NIKOLA_LOG_ERROR("Failed to read the dialogue file at \'%s\'", txt_path.c_str());
    return;
  }

  // Read the whole file into the string
  nikola::String dialogue; 
  nikola::file_read_string(file, &dialogue);

  // Fill the lines array
  nikola::String line = ""; 
  for(auto& ch : dialogue) {
    if(ch != '*') {
      line += ch;
      continue;
    }

    s_manager.lines.push_back(line);
    line = "";
  }

  file.close();
  NIKOLA_LOG_TRACE("Loaded dialogue at \'%s\'", txt_path.c_str());
}

const nikola::String& dialogue_manager_advance() {
  s_manager.current_line++;
  return s_manager.lines[s_manager.current_line - 1];
}

/// Dialogue manager functions
/// ----------------------------------------------------------------------
