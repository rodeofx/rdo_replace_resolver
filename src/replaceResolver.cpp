// Copyright 2019 Rodeo FX.  All rights reserved.
#include "debugCodes.h"
#include "replaceResolver.h"
#include "replaceResolverContext.h"
#include "tokens.h"

#include <pxr/base/arch/fileSystem.h>
#include <pxr/base/arch/systemInfo.h>
#include <pxr/base/js/json.h>
#include <pxr/base/tf/fileUtils.h>
#include <pxr/base/tf/getenv.h>
#include <pxr/base/tf/pathUtils.h>
#include <pxr/base/tf/staticData.h>
#include <pxr/base/tf/stringUtils.h>
#include <pxr/base/vt/value.h>
#include <pxr/usd/ar/assetInfo.h>
#include <pxr/usd/ar/defineResolver.h>
#include <pxr/usd/ar/filesystemAsset.h>
#include <pxr/usd/ar/resolverContext.h>
#include <pxr/usd/sdf/layer.h>

#include <tbb/concurrent_hash_map.h>
#include <fstream>

PXR_NAMESPACE_OPEN_SCOPE

AR_DEFINE_RESOLVER(ReplaceResolver, ArResolver);


namespace {

bool _GetReplacePairsFromUsdFile(const std::string& filePath, ReplaceResolverContext& context)
{
    bool found = false;
    auto layer = SdfLayer::FindOrOpen(TfAbsPath(filePath));
    if (layer) {
        auto layerMetaData = layer->GetMetadata();
        auto rootId = SdfAbstractDataSpecId(&SdfPath::AbsoluteRootPath());
        auto replaceData = layerMetaData->Get(rootId, SdfFieldKeys->CustomLayerData);

        if (!replaceData.IsEmpty()) {
            TF_DEBUG(REPLACERESOLVER_REPLACE).Msg("Replace metadata found in file: \"%s\"\n",
                                                filePath.c_str());

            VtDictionary dic = replaceData.Get<VtDictionary>();
            auto it = dic.find(ReplaceResolverTokens->replacePairs);
            if(it != dic.end()) {
                VtValue allPairsValue =  dic[ReplaceResolverTokens->replacePairs];
                VtStringArray allPairs = allPairsValue.Get<VtStringArray>();
                if(allPairs.size() > 0)
                {
                    found = true;
                    for (size_t i = 0; i < allPairs.size(); i+=2) {
                        context.AddReplacePair(allPairs[i], allPairs[i+1]);
                    }
                }
            }
        }
    }
    return found;
}

bool _GetReplacePairsFromJsonFile(const std::string& filePath, ReplaceResolverContext& context)
{
    bool found = false;
    // Check if there is a "replace file" in the directory
    std::string assetDir = TfGetPathName(TfAbsPath(filePath));
    std::string replaceFilePath = TfNormPath(
        TfStringCatPaths(assetDir, ReplaceResolverTokens->replaceFileName));
    
    std::ifstream ifs(replaceFilePath);

    // Try to find replace pairs in json file
    if (ifs) {
        TF_DEBUG(REPLACERESOLVER_REPLACE).Msg("Replace file found: \"%s\"\n", 
                                            replaceFilePath.c_str());

        JsParseError error;
        const JsValue value = JsParseStream(ifs, &error);
        ifs.close();

        if (!value.IsNull() && value.IsArray()) {
            if (value.GetJsArray().size() > 0) {
                found = true;
                for(const auto& pair : value.GetJsArray()) {
                    if(pair.IsArray()) {
                        context.AddReplacePair(
                            pair.GetJsArray()[0].GetString(), pair.GetJsArray()[1].GetString());
                    }
                }
            }
        }
        else {
            fprintf(stderr, "Error: parse error at %s:%d:%d: %s\n",
                replaceFilePath.c_str(), error.line, error.column, error.reason.c_str());
        }
    }

    return found;  
}

bool _IsFileRelative(const std::string& path) {
    return path.find("./") == 0 || path.find("../") == 0;
}

TfStaticData<std::vector<std::string>> _SearchPath;

} // end anonymous namespace

std::vector<std::string> _GetSearchPaths() 
{
    std::vector<std::string> searchPath = *_SearchPath;

    const std::string envPath = TfGetenv("PXR_AR_DEFAULT_SEARCH_PATH");
    if (!envPath.empty()) {
        const std::vector<std::string> envSearchPath = 
            TfStringTokenize(envPath, ARCH_PATH_LIST_SEP);
        searchPath.insert(
            searchPath.end(), envSearchPath.begin(), envSearchPath.end());
    }

    return searchPath;
}

struct ReplaceResolver::_Cache
{
    using _PathToResolvedPathMap = 
        tbb::concurrent_hash_map<std::string, std::string>;
    _PathToResolvedPathMap _pathToResolvedPathMap;
};

ReplaceResolver::ReplaceResolver()
{
    _fallbackContext = ReplaceResolverContext(_GetSearchPaths());
}

ReplaceResolver::~ReplaceResolver()
{
}

void
ReplaceResolver::SetDefaultSearchPath(
    const std::vector<std::string>& searchPath)
{
    *_SearchPath = searchPath;
}

void
ReplaceResolver::ConfigureResolverForAsset(const std::string& path)
{
    _defaultContext = CreateDefaultContextForAsset(path);
}

bool
ReplaceResolver::IsRelativePath(const std::string& path)
{
    return (!path.empty() && TfIsRelativePath(path));
}

bool
ReplaceResolver::IsRepositoryPath(const std::string& path)
{
    return false;
}

std::string
ReplaceResolver::AnchorRelativePath(
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
ReplaceResolver::IsSearchPath(const std::string& path)
{
    return IsRelativePath(path) && !_IsFileRelative(path);
}

std::string
ReplaceResolver::GetExtension(const std::string& path)
{
    return TfGetExtension(path);
}

std::string
ReplaceResolver::ComputeNormalizedPath(const std::string& path)
{
    return TfNormPath(path);
}

std::string
ReplaceResolver::ComputeRepositoryPath(const std::string& path)
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

std::string _ReplaceFromContext(const ReplaceResolverContext& ctx, const std::string& path)

{
    std::string result = path;

    auto oldAndNewStrings = ctx.GetReplaceMap();
    for (auto it = oldAndNewStrings.begin(); it != oldAndNewStrings.end(); ++it)
    {

        std::size_t found = path.find(it->first);
        if(found != std::string::npos) {
            result.replace(found, it->first.size(), it->second);
            break;
        }
    }

    return result;
}

std::string
ReplaceResolver::_ResolveNoCache(const std::string& path)
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
            auto currentContext = _GetCurrentContext();
            if(currentContext) {
                TF_DEBUG(REPLACERESOLVER_CURRENTCONTEXT).Msg(
                    "ReplaceResolverContext: \"%s\"\n",
                    ArResolverContext(*currentContext).GetDebugString().c_str());
            }
            const ReplaceResolverContext* contexts[2] =
                {currentContext, &_fallbackContext};
            for (const ReplaceResolverContext* ctx : contexts) {
                if (ctx) {
                    // Replace sub strings from context old/new pairs.
                    std::string replacedPath = _ReplaceFromContext(*ctx, path);

                    for (const auto& searchPath : ctx->GetSearchPath()) {
                        resolvedPath = _Resolve(searchPath, replacedPath);
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
ReplaceResolver::Resolve(const std::string& path)
{
    return ResolveWithAssetInfo(path, /* assetInfo = */ nullptr);
}

std::string
ReplaceResolver::ResolveWithAssetInfo(
    const std::string& path, 
    ArAssetInfo* assetInfo)
{
    TF_DEBUG(REPLACERESOLVER_PATH).Msg("Unresolved path \"%s\"\n",
                                      path.c_str());

    if (path.empty()) {
        return path;
    }

    std::string resolvedPath;
    if (_CachePtr currentCache = _GetCurrentCache()) {
        _Cache::_PathToResolvedPathMap::accessor accessor;
        if (currentCache->_pathToResolvedPathMap.insert(
                accessor, std::make_pair(path, std::string()))) {
            accessor->second = _ResolveNoCache(path);
        }
        resolvedPath = accessor->second;
    }

    if (resolvedPath.empty()) {
        resolvedPath = _ResolveNoCache(path);
    }

    TF_DEBUG(REPLACERESOLVER_PATH).Msg("Resolved path \"%s\"\n",
                                      resolvedPath.c_str());
    return resolvedPath;
}

std::string
ReplaceResolver::ComputeLocalPath(const std::string& path)
{
    return path.empty() ? path : TfAbsPath(path);
}

void
ReplaceResolver::UpdateAssetInfo(
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
ReplaceResolver::GetModificationTimestamp(
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
ReplaceResolver::FetchToLocalResolvedPath(
    const std::string& path,
    const std::string& resolvedPath)
{
    // ReplaceResolver always resolves paths to a file on the
    // local filesystem. Because of this, we know the asset specified 
    // by the given path already exists on the filesystem at 
    // resolvedPath, so no further data fetching is needed.
    return true;
}

std::shared_ptr<ArAsset> 
ReplaceResolver::OpenAsset(
    const std::string& resolvedPath)
{
    FILE* f = ArchOpenFile(resolvedPath.c_str(), "rb");
    if (!f) {
        return nullptr;
    }

    return std::shared_ptr<ArAsset>(new ArFilesystemAsset(f));
}

bool
ReplaceResolver::CanWriteLayerToPath(
    const std::string& path,
    std::string* whyNot)
{
    return true;
}

bool
ReplaceResolver::CanCreateNewLayerWithIdentifier(
    const std::string& identifier, 
    std::string* whyNot)
{
    return true;
}

ArResolverContext 
ReplaceResolver::CreateDefaultContext()
{
    return _defaultContext;
}

ArResolverContext 
ReplaceResolver::CreateDefaultContextForAsset(
    const std::string& filePath)
{
    if (filePath.empty()){
        return ArResolverContext(ReplaceResolverContext());
    }

    auto context = ReplaceResolverContext(_GetSearchPaths());
    
    // Find replace pairs in SdfLayer metadata of this filePath
    std::string extension = TfGetExtension(filePath);
    if(extension == "usd" || extension == "usda" || extension == "usdc") {
        _GetReplacePairsFromUsdFile(filePath, context);
    }

    // If the is a json file at the same location we allow adding 
    // or overriding replace pairs.
    _GetReplacePairsFromJsonFile(filePath, context);

    return ArResolverContext(context);
}

void 
ReplaceResolver::RefreshContext(const ArResolverContext& context)
{
}

ArResolverContext
ReplaceResolver::GetCurrentContext()
{
    const ReplaceResolverContext* ctx = _GetCurrentContext();
    return ctx ? ArResolverContext(*ctx) : ArResolverContext();
}

void 
ReplaceResolver::BeginCacheScope(
    VtValue* cacheScopeData)
{
    _threadCache.BeginCacheScope(cacheScopeData);
}

void 
ReplaceResolver::EndCacheScope(
    VtValue* cacheScopeData)
{
    _threadCache.EndCacheScope(cacheScopeData);
}

ReplaceResolver::_CachePtr 
ReplaceResolver::_GetCurrentCache()
{
    return _threadCache.GetCurrentCache();
}

void 
ReplaceResolver::BindContext(
    const ArResolverContext& context,
    VtValue* bindingData)
{
    const ReplaceResolverContext* ctx = 
        context.Get<ReplaceResolverContext>();

    if (!context.IsEmpty() && !ctx) {
        TF_CODING_ERROR(
            "Unknown resolver context object: %s", 
            context.GetDebugString().c_str());
    }

    _ContextStack& contextStack = _threadContextStack.local();
    contextStack.push_back(ctx);
}

void 
ReplaceResolver::UnbindContext(
    const ArResolverContext& context,
    VtValue* bindingData)
{
    _ContextStack& contextStack = _threadContextStack.local();
    if (contextStack.empty() ||
        contextStack.back() != context.Get<ReplaceResolverContext>()) {
        TF_CODING_ERROR(
            "Unbinding resolver context in unexpected order: %s",
            context.GetDebugString().c_str());
    }

    if (!contextStack.empty()) {
        contextStack.pop_back();
    }
}

const ReplaceResolverContext* 
ReplaceResolver::_GetCurrentContext()
{
    _ContextStack& contextStack = _threadContextStack.local();
    return contextStack.empty() ? nullptr : contextStack.back();
}

PXR_NAMESPACE_CLOSE_SCOPE
