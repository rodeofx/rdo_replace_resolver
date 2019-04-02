#include "resolver.h"

#include <pxr/usd/ar/defineResolver.h>

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

using namespace std;

AR_DEFINE_RESOLVER(UsdHelloResolver, ArResolver)

UsdHelloResolver::UsdHelloResolver() : ArDefaultResolver() {
  cout << "UsdHelloResolver::UsdHelloResolver" << endl;
}

UsdHelloResolver::~UsdHelloResolver() {
  cout << "UsdHelloResolver::~UsdHelloResolver" << endl;
}

string UsdHelloResolver::Resolve(const string& path) {
  cout << "UsdHelloResolver::Resolve " << path << endl;
  return ArDefaultResolver::Resolve(path);
}

PXR_NAMESPACE_CLOSE_SCOPE
