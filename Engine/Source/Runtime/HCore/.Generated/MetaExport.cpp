#include "pch.h"
#include "MetaExport.h"
#include <HCore/Include/Meta/ReflectionDatabase.h>
#include "Meta.Generated.h"

namespace Horizon
{
	void ImportHCoreMeatData()
	{
		MetaGenerated::module::ModuleHCoreMeta mz(Meta::ReflectionDatabase::Instance());
	}
}


