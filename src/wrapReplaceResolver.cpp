// Copyright 2019 Rodeo FX.  All rights reserved.
#include "replaceResolver.h"

#include "boost_include_wrapper.h"

#include <pxr/pxr.h>

#include BOOST_INCLUDE(python/class.hpp)

using namespace BOOST_NAMESPACE::python;

PXR_NAMESPACE_USING_DIRECTIVE

void
wrapReplaceResolver()
{
    using This = ReplaceResolver;

    class_<This, bases<ArResolver>, BOOST_NAMESPACE::noncopyable>
        ("ReplaceResolver", no_init)

        .def("SetDefaultSearchPath", &This::SetDefaultSearchPath,
             args("searchPath"))
        .staticmethod("SetDefaultSearchPath")
        ;
}
