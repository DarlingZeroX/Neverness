
#include "MetaExport.h"
#include <NNCore/Include/Meta/ReflectionDatabase.h>
#include "Meta.Generated.h"

namespace NN::Core
{
	void ImportHCoreMeatData()
	{
		MetaGenerated::module::ModuleHCoreMeta mz(Meta::ReflectionDatabase::Instance());
	}
}


