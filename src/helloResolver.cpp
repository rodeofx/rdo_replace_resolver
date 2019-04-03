#include "helloResolver.h"
#include "helloResolverContext.h"

#include <pxr/usd/ar/defineResolver.h>
#include <pxr/usd/ar/filesystemAsset.h>
#include <pxr/usd/ar/assetInfo.h>
#include <pxr/usd/ar/resolverContext.h>

#include <pxr/base/arch/fileSystem.h>
#include <pxr/base/arch/systemInfo.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/base/tf/fileUtils.h>
#include <pxr/base/tf/pathUtils.h>
#include <pxr/base/tf/staticData.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/base/vt/value.h>

#include <tbb/concurrent_hash_map.h>

PXR_NAMESPACE_OPEN_SCOPE

AR_DEFINE_RESOLVER(HelloResolver, ArResolver);

static bool
_IsFileRelative(const std::string& path) {
    return path.find("./") == 0 || path.find("../") == 0;
}

static TfStaticData<std::vector<std::string>> _SearchPath;

struct HelloResolver::_Cache
{
    using _PathToResolvedPathMap = 
        tbb::concurrent_hash_map<std::string, std::string>;
    _PathToResolvedPathMap _pathToResolvedPathMap;
};

HelloResolver::HelloResolver()
{
    std::vector<std::string> searchPath = *_SearchPath;

    const std::string envPath = TfGetenv("PXR_AR_DEFAULT_SEARCH_PATH");
    if (!envPath.empty()) {
        const std::vector<std::string> envSearchPath = 
            TfStringTokenize(envPath, ARCH_PATH_LIST_SEP);
        searchPath.insert(
            searchPath.end(), envSearchPath.begin(), envSearchPath.end());
    }

    _fallbackContext = HelloResolverContext(searchPath);
}

HelloResolver::~HelloResolver()
{
}

void
HelloResolver::SetDefaultSearchPath(
    const std::vector<std::string>& searchPath)
{
    *_SearchPath = searchPath;
}

void
HelloResolver::ConfigureResolverForAsset(const std::string& path)
{
    _defaultContext = CreateDefaultContextForAsset(path);
}

bool
HelloResolver::IsRelativePath(const std::string& path)
{
    return (!path.empty() && TfIsRelativePath(path));
}

bool
HelloResolver::IsRepositoryPath(const std::string& path)
{
    return false;
}

std::string
HelloResolver::AnchorRelativePath(
    const std::string& anchorPath, 
    const std::string& path)
{
    if (TfIsRelativePath(anchorPath) ||
        !IsRelativePath(path)) {
        return path;
    }

    // Ensure we are using forward slashes and not back slashes.
    std::string forwardPath = anchorPath;
    std::replace(forwardPath.begin(), forwardPath.end(), '\\', '/');

    // If anchorPath does not end with a '/', we assume it is specifying
    // a file, strip off the last component, and anchor the path to that
    // directory.
    const std::string anchoredPath = TfStringCatPaths(
        TfStringGetBeforeSuffix(forwardPath, '/'), path);
    return TfNormPath(anchoredPath);
}

bool
HelloResolver::IsSearchPath(const std::string& path)
{
    return IsRelativePath(path) && !_IsFileRelative(path);
}

std::string
HelloResolver::GetExtension(const std::string& path)
{
    return TfGetExtension(path);
}

std::string
HelloResolver::ComputeNormalizedPath(const std::string& path)
{
    return TfNormPath(path);
}

std::string
HelloResolver::ComputeRepositoryPath(const std::string& path)
{
    return std::string();
}

static std::string
_Resolve(
    const std::string& anchorPath,
    const std::string& path)
{
    std::string resolvedPath = path;
    if (!anchorPath.empty()) {
        // XXX - CLEANUP:
        // It's tempting to use AnchorRelativePath to combine the two
        // paths here, but that function's file-relative anchoring
        // causes consumers to break. 
        // 
        // Ultimately what we should do is specify whether anchorPath 
        // in both Resolve and AnchorRelativePath can be files or directories 
        // and fix up all the callers to accommodate this.
        resolvedPath = TfStringCatPaths(anchorPath, path);
    }
    return TfPathExists(resolvedPath) ? resolvedPath : std::string();
}

std::string
HelloResolver::_ResolveNoCache(const std::string& path)
{
    if (path.empty()) {
        return path;
    }

    if (IsRelativePath(path)) {
        // First try to resolve relative paths against the current
        // working directory.
        std::string resolvedPath = _Resolve(ArchGetCwd(), path);
        if (!resolvedPath.empty()) {
            return resolvedPath;
        }

        // If that fails and the path is a search path, try to resolve
        // against each directory in the specified search paths.
        if (IsSearchPath(path)) {
            const HelloResolverContext* contexts[2] =
                {_GetCurrentContext(), &_fallbackContext};
            for (const HelloResolverContext* ctx : contexts) {
                if (ctx) {
                    for (const auto& searchPath : ctx->GetSearchPath()) {
                        resolvedPath = _Resolve(searchPath, path);
                        if (!resolvedPath.empty()) {
                            return resolvedPath;
                        }
                    }
                }
            }
        }

        return std::string();
    }

    return _Resolve(std::string(), path);
}

std::string
HelloResolver::Resolve(const std::string& path)
{
    return ResolveWithAssetInfo(path, /* assetInfo = */ nullptr);
}

std::string
HelloResolver::ResolveWithAssetInfo(
    const std::string& path, 
    ArAssetInfo* assetInfo)
{
    if (path.empty()) {
        return path;
    }

    if (_CachePtr currentCache = _GetCurrentCache()) {
        _Cache::_PathToResolvedPathMap::accessor accessor;
        if (currentCache->_pathToResolvedPathMap.insert(
                accessor, std::make_pair(path, std::string()))) {
            accessor->second = _ResolveNoCache(path);
        }
        return accessor->second;
    }

    return _ResolveNoCache(path);
}

std::string
HelloResolver::ComputeLocalPath(const std::string& path)
{
    return path.empty() ? path : TfAbsPath(path);
}

void
HelloResolver::UpdateAssetInfo(
    const std::string& identifier,
    const std::string& filePath,
    const std::string& fileVersion,
    ArAssetInfo* resolveInfo)
{
    if (resolveInfo) {
        if (!fileVersion.empty()) {
            resolveInfo->version = fileVersion;
        }
    }
}

VtValue
HelloResolver::GetModificationTimestamp(
    const std::string& path,
    const std::string& resolvedPath)
{
    // Since the default resolver always resolves paths to local
    // paths, we can just look at the mtime of the file indicated
    // by resolvedPath.
    double time;
    if (ArchGetModificationTime(resolvedPath.c_str(), &time)) {
        return VtValue(time);
    }
    return VtValue();
}

bool 
HelloResolver::FetchToLocalResolvedPath(
    const std::string& path,
    const std::string& resolvedPath)
{
    // HelloResolver always resolves paths to a file on the
    // local filesystem. Because of this, we know the asset specified 
    // by the given path already exists on the filesystem at 
    // resolvedPath, so no further data fetching is needed.
    return true;
}

std::shared_ptr<ArAsset> 
HelloResolver::OpenAsset(
    const std::string& resolvedPath)
{
    FILE* f = ArchOpenFile(resolvedPath.c_str(), "rb");
    if (!f) {
        return nullptr;
    }

    return std::shared_ptr<ArAsset>(new ArFilesystemAsset(f));
}

bool
HelloResolver::CanWriteLayerToPath(
    const std::string& path,
    std::string* whyNot)
{
    return true;
}

bool
HelloResolver::CanCreateNewLayerWithIdentifier(
    const std::string& identifier, 
    std::string* whyNot)
{
    return true;
}

ArResolverContext 
HelloResolver::CreateDefaultContext()
{
    return _defaultContext;
}

ArResolverContext 
HelloResolver::CreateDefaultContextForAsset(
    const std::string& filePath)
{
    if (filePath.empty()){
        return ArResolverContext(HelloResolverContext());
    }

    std::string assetDir = TfGetPathName(TfAbsPath(filePath));
    
    return ArResolverContext(HelloResolverContext(
                                 std::vector<std::string>(1, assetDir)));
}

void 
HelloResolver::RefreshContext(const ArResolverContext& context)
{
}

ArResolverContext
HelloResolver::GetCurrentContext()
{
    const HelloResolverContext* ctx = _GetCurrentContext();
    return ctx ? ArResolverContext(*ctx) : ArResolverContext();
}

void 
HelloResolver::BeginCacheScope(
    VtValue* cacheScopeData)
{
    _threadCache.BeginCacheScope(cacheScopeData);
}

void 
HelloResolver::EndCacheScope(
    VtValue* cacheScopeData)
{
    _threadCache.EndCacheScope(cacheScopeData);
}

HelloResolver::_CachePtr 
HelloResolver::_GetCurrentCache()
{
    return _threadCache.GetCurrentCache();
}

void 
HelloResolver::BindContext(
    const ArResolverContext& context,
    VtValue* bindingData)
{
    const HelloResolverContext* ctx = 
        context.Get<HelloResolverContext>();

    if (!context.IsEmpty() && !ctx) {
        TF_CODING_ERROR(
            "Unknown resolver context object: %s", 
            context.GetDebugString().c_str());
    }

    _ContextStack& contextStack = _threadContextStack.local();
    contextStack.push_back(ctx);
}

void 
HelloResolver::UnbindContext(
    const ArResolverContext& context,
    VtValue* bindingData)
{
    _ContextStack& contextStack = _threadContextStack.local();
    if (contextStack.empty() ||
        contextStack.back() != context.Get<HelloResolverContext>()) {
        TF_CODING_ERROR(
            "Unbinding resolver context in unexpected order: %s",
            context.GetDebugString().c_str());
    }

    if (!contextStack.empty()) {
        contextStack.pop_back();
    }
}

const HelloResolverContext* 
HelloResolver::_GetCurrentContext()
{
    _ContextStack& contextStack = _threadContextStack.local();
    return contextStack.empty() ? nullptr : contextStack.back();
}

PXR_NAMESPACE_CLOSE_SCOPE
