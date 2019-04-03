#include "helloResolverContext.h"

#include <pxr/pxr.h>

#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/hash.h>
#include <pxr/base/tf/ostreamMethods.h>
#include <pxr/base/tf/pathUtils.h>
#include <pxr/base/tf/stringUtils.h>

#include <boost/functional/hash.hpp>

PXR_NAMESPACE_OPEN_SCOPE

HelloResolverContext::HelloResolverContext(
    const std::vector<std::string>& searchPath)
{
    _searchPath.reserve(searchPath.size());
    for (const std::string& p : searchPath) {
        if (p.empty()) {
            continue;
        }

        const std::string absPath = TfAbsPath(p);
        if (absPath.empty()) {
            TF_WARN(
                "Could not determine absolute path for search path prefix "
                "'%s'", p.c_str());
            continue;
        }

        _searchPath.push_back(absPath);
    }
}

bool
HelloResolverContext::operator<(const HelloResolverContext& rhs) const
{
    return _searchPath < rhs._searchPath;
}

bool 
HelloResolverContext::operator==(const HelloResolverContext& rhs) const
{
    return _searchPath == rhs._searchPath;
}

bool 
HelloResolverContext::operator!=(const HelloResolverContext& rhs) const
{
    return !(*this == rhs);
}

std::string 
HelloResolverContext::GetAsString() const
{
    std::string result = "Search path: ";
    if (_searchPath.empty()) {
        result += "[ ]";
    }
    else {
        result += "[\n    ";
        result += TfStringJoin(_searchPath, "\n    ");
        result += "\n]";
    }
    return result;
}

size_t 
hash_value(const HelloResolverContext& context)
{
    size_t hash = 0;
    for (const std::string& p : context.GetSearchPath()) {
        boost::hash_combine(hash, TfHash()(p));
    }
    return hash;
}

PXR_NAMESPACE_CLOSE_SCOPE
