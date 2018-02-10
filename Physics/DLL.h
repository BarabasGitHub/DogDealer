#ifdef DogDealerPhysics_DLL_EXPORT
#define PHYSICS_DLL __declspec(dllexport)
#else                            
#define PHYSICS_DLL __declspec(dllimport)
#endif                        