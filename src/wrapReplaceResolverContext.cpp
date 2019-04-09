
#include "replaceResolverContext.h"

#include <boost/python/class.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/return_by_value.hpp>
#include <boost/python/return_value_policy.hpp>

#include <pxr/pxr.h>
#include <pxr/usd/ar/pyResolverContext.h>
#include <pxr/base/tf/pyUtils.h>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

static std::string
_Repr(const ReplaceResolverContext& ctx)
{
    std::string repr = TF_PY_REPR_PREFIX;
    repr += "ReplaceResolverContext(";
    repr += ctx.GetAsString();
    repr += ")";
    return repr;
}

static size_t
_Hash(const ReplaceResolverContext& ctx)
{
    return hash_value(ctx);
}

void
wrapReplaceResolverContext()
{
    using This = ReplaceResolverContext;

    class_<This>
        ("ReplaceResolverContext", no_init)
        .def(init<>())
        .def(init<const std::vector<std::string>&>(
                arg("searchPaths")))

        .def(self == self)
        .def(self != self)

        .def("GetSearchPath", &This::GetSearchPath,
             return_value_policy<return_by_value>())

        .def("AddReplacePair", &This::AddReplacePair,
             return_value_policy<return_by_value>())

        .def("__str__", &This::GetAsString)
        .def("__repr__", &_Repr)
        .def("__hash__", &_Hash)
        ;

    ArWrapResolverContextForPython<This>();
}
