
#include "MetaExport.h"
#include <NNKernel/Include/Meta/ReflectionDatabase.h>
#include "Meta.Generated.h"

namespace Horizon
{
	void ImportHCoreMeatData()
	{
		MetaGenerated::module::ModuleHCoreMeta mz(Meta::ReflectionDatabase::Instance());
	}
}


