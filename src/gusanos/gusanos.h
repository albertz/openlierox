#ifndef VERMES_VERMES_H
#define VERMES_VERMES_H

bool gusInitBase();
bool gusInit(const std::string& mod);
bool gusCanRunFrame();
void gusLogicFrame(); // always called
void gusRenderFrameMenu();
void gusQuit();

#endif //VERMES_VERMES_H

