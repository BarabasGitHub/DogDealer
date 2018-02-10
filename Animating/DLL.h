#ifdef DogDealerAnimating_DLL_EXPORT
#define ANIMATING_DLL __declspec(dllexport)
#else                            
#define ANIMATING_DLL __declspec(dllimport)
#endif                        