#ifdef DogDealerGameLogic_DLL_EXPORT
#define GAMELOGIC_DLL __declspec(dllexport)
#else                            
#define GAMELOGIC_DLL __declspec(dllimport)
#endif                        