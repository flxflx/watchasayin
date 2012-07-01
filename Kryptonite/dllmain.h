// dllmain.h : Declaration of module class.

class CKryptoniteModule : public ATL::CAtlDllModuleT< CKryptoniteModule >
{
public :
	DECLARE_LIBID(LIBID_KryptoniteLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_KRYPTONITE, "{91923BA4-1E69-4765-B146-DE0DED5F6E61}")
};

extern class CKryptoniteModule _AtlModule;
