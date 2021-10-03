#pragma once

#include <weapon/ArsenalBuilder.h>

class DODArsenalBuilder: public ArsenalBuilder {

public:
	DODArsenalBuilder();

private:
	  template<typename _Tp, typename... _Args>
	  void addPair(const char* name1, const char* name2, _Args&&... args);
};
