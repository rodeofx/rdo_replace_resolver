#include "debugCodes.h"

#include <pxr/pxr.h>
#include <pxr/base/tf/debug.h>
#include <pxr/base/tf/registryManager.h>

PXR_NAMESPACE_OPEN_SCOPE


TF_REGISTRY_FUNCTION(TfDebug)
{
    TF_DEBUG_ENVIRONMENT_SYMBOL(HELLORESOLVER_PATH, "Print debug output during path resolution");
    TF_DEBUG_ENVIRONMENT_SYMBOL(HELLORESOLVER_REPLACE, "Print debug output during replace operation");
    TF_DEBUG_ENVIRONMENT_SYMBOL(HELLORESOLVER_CURRENTCONTEXT, "Print debug output on current context");
}

PXR_NAMESPACE_CLOSE_SCOPE

