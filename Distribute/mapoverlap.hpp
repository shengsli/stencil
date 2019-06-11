/**
 * mapoverlap.hpp
 * \class: 
 * \author: sheng
 * \version: 0.0.0
 * \date: 2019-03-14 Thu 14:14
 */

#progma once

namespace impl
{
	template<typename, typename, typename...>
	class MapOverlap1D;
}


namespace impl
{
	template<typename In, typename Out, typename... Args>
	class MapOverlap1D
	{
	public:
		void setBackEnd(BackendSpec) {}
		void resetBackend() {}
		void setExecPlan(ExecPlan *plan)
		{
			delete plan;
		}
		template<typename... Args>
		void tune(Args&&... args) {}
	};
}
