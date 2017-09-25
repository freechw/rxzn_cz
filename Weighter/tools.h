/*
 */
 
#ifndef TOOLS_H_
#define TOOLS_H_

#define SET_FLG(f)     ((f) = 1)
#define CLR_FLG(f)     ((f) = 0)

inline INT8U IsSet(INT8U flag){
	return (1 == flag);
}

inline INT8U IsClr(INT8U flag){
	return (0 == flag);
}

#endif /*TOOLS_H_*/
