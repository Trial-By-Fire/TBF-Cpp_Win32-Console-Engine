// Parent Header
#include "String.hpp"







ptr<ro WideChar> operator""_wc(ptr<ro wchar_t> _um, uIntDM /*_umSize*/)
{
	return RCast<ro WideChar>(_um);
}