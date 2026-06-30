#pragma once

class Model;

// Returns a unix-style success code.
// Upon failure, prints message to stderr.
int RunScript(const char* script, Model const& model);
