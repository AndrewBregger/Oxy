#include "io.h"
#include <cassert>

std::string get_file_from_path(const std::string& path) {
  u64 last_slash = path.find_last_of('/');
  if(last_slash != std::string::npos)
    return path.substr(last_slash + 1);
  else 
    return path;
}

std::string get_file_dir(const std::string& path) {
  u64 last_slash = path.find_last_of('/');
  // handle special case "/file.pr"
  if(path[0] == '/' and last_slash == 0)
    return "/";
  if(last_slash != std::string::npos)
    return path.substr(0, last_slash);
  else 
    return path;
}

void load_lines(File* file);

File load_file(const std::string& path) {
  File file;

  FILE* f = fopen(path.c_str(), "r");
  fseek(f, 0, SEEK_END);

  u64 size = ftell(f);

  char* temp = new char[size + 1];
  rewind(f);
  fread(temp, 1, size, f);
  temp[size] = '\0';
  
  file.content = temp;
  file.filename = get_file_from_path(path);
  file.path = get_file_dir(path);

  load_lines(&file);

  return file;
}

std::string get_line(File* file, u32 line) {
  assert(line - 1 >= 0);
  std::pair<u32, u32> l = file->lines[line - 1];
  return file->content.substr(l.first, l.second);
}

void load_lines(File* file) {
	u32 start = 0;
	u32 i = 0;
	for(i = 0; i < file->content.size(); ++i) {
		if(file->content[i] == '\n') {
			file->lines.push_back({start, i - start});
			start = i;
		}
	}
	file->lines.push_back({start, file->content.size() - start});
}
