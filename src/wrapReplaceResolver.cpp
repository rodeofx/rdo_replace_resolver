#include "replaceResolver.h"

#include <pxr/pxr.h>

#include <boost/python/class.hpp>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

void
wrapReplaceResolver()
{
    using This = ReplaceResolver;

    class_<This, bases<ArResolver>, boost::noncopyable>
        ("ReplaceResolver", no_init)

        .def("SetDefaultSearchPath", &This::SetDefaultSearchPath,
             args("searchPath"))
        .staticmethod("SetDefaultSearchPath")
        ;
}
