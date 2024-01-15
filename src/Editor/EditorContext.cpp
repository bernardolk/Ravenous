#include "Editor/EditorContext.h"

namespace Editor
{
	void REditorContext::RemoveFromDeletedEntityList(RUUID ID)
	{
		std::vector<RUUID>::iterator It = DeletionLog.begin();
		while (It != DeletionLog.end()) {
			if (*It == ID) {
				DeletionLog.erase(It);
				return;
			}
		}
	}
}