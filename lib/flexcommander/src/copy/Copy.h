#pragma once

#include <stdint.h>
#include <Flexcommander.h>

void MakePath(char* dir);
void CopyFile(const char* dest, const char* filename, HFSPlusCatalogFile file, FlexCommanderFS *fs);