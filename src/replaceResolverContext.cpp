// Copyright 2019 Rodeo FX.  All rights reserved.
#include "replaceResolverContext.h"

#include "boost_include_wrapper.h"

#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/hash.h>
#include <pxr/base/tf/ostreamMethods.h>
#include <pxr/base/tf/pathUtils.h>
#include <pxr/base/tf/stringUtils.h>

#include BOOST_INCLUDE(functional/hash.hpp)

PXR_NAMESPACE_OPEN_SCOPE

ReplaceResolverContext::ReplaceResolverContext(
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

void ReplaceResolverContext::AddReplacePair(const std::string& oldStr, const std::string& newStr)
{
    _oldAndNewStrings.emplace(std::piecewise_construct,
        std::forward_as_tuple(oldStr),
        std::forward_as_tuple(newStr));
}

bool
ReplaceResolverContext::operator<(const ReplaceResolverContext& rhs) const
{
    bool result = _searchPath < rhs._searchPath;

    if (result == true) {
        result = _oldAndNewStrings.size() < rhs._oldAndNewStrings.size();
    }

    return result;
}

bool 
ReplaceResolverContext::operator==(const ReplaceResolverContext& rhs) const
{
    bool result = _searchPath == rhs._searchPath;

    if(result == true) {
        result = _oldAndNewStrings.size() == rhs._oldAndNewStrings.size();
    }

    return result;
}

bool 
ReplaceResolverContext::operator!=(const ReplaceResolverContext& rhs) const
{
    return !(*this == rhs);
}

std::string 
ReplaceResolverContext::GetAsString() const
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

    if( _oldAndNewStrings.size() > 0) {
        result += "\nOld to new token: ";
        result += "[";
        for (auto it = _oldAndNewStrings.begin(); it != _oldAndNewStrings.end(); ++it) 
        {
            result += "\n    " + it->first + ": ";
            result += it->second;
        }
        result += "\n]";
    }
    return result;
}

size_t 
hash_value(const ReplaceResolverContext& context)
{
    size_t hash = 0;
    for (const std::string& p : context.GetSearchPath()) {
        BOOST_NAMESPACE::hash_combine(hash, TfHash()(p));
    }

    auto replaceMap = context.GetReplaceMap();
    for (auto it = replaceMap.begin(); it != replaceMap.end(); ++it) 
    {
        BOOST_NAMESPACE::hash_combine(hash, TfHash()(it->first));
        BOOST_NAMESPACE::hash_combine(hash, TfHash()(it->second));
    }
    return hash;
}

PXR_NAMESPACE_CLOSE_SCOPE
