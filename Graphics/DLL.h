#ifdef DogDealerGraphics_DLL_EXPORT
#define GRAPHICS_DLL __declspec(dllexport)
#else                            
#define GRAPHICS_DLL __declspec(dllimport)
#endif                        