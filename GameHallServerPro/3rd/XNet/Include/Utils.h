#ifndef _CTOOLS_H_
#define _CTOOLS_H_
#include <string>
#include <vector>

// 单列宏 禁止拷贝构造函数
#define SINGLETON_CLASS(className) \
public: \
	static className* Instance() \
	{ \
		static className instance; \
		return &instance; \
	} \
	className(const className&) = delete; \
	className& operator=(const className&) = delete; 


namespace X
{

	namespace Utils
	{
		/**
		 * @描述 生成UUID
		 *
		 */
		std::string GeneratorUUID();

		size_t GeneratorUUIDNumber();
	}
}


#endif
