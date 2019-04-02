#ifndef USD_HELLO_RESOLVER_H
#define USD_HELLO_RESOLVER_H

#include <pxr/usd/ar/defaultResolver.h>

PXR_NAMESPACE_OPEN_SCOPE

class UsdHelloResolver : public ArDefaultResolver {
public:
  UsdHelloResolver();
  ~UsdHelloResolver() override;

  std::string Resolve(const std::string& path) override;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // USD_HELLO_RESOLVER_H