#pragma once

#include <nikola/nikola.h>

/// ----------------------------------------------------------------------
/// Dialogue manager functions

void dialogue_manager_init(const nikola::FilePath& txt_path);

const nikola::String& dialogue_manager_advance();

/// Dialogue manager functions
/// ----------------------------------------------------------------------
