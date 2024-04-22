#ifndef LEVEL_EDITOR_H
#define LEVEL_EDITOR_H

struct level_editor {
};

struct level_editor_create_info {
    window* Window;
};

bool Level_Editor_Init(level_editor* LevelEditor, const level_editor_create_info& CreateInfo);
bool Level_Editor_Is_Open(level_editor* LevelEditor);
void Level_Editor_Update(level_editor* LevelEditor);
void Level_Editor_Shutdown(level_editor* LevelEditor);

#endif