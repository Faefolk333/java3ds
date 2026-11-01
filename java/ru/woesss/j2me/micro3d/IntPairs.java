package ru.woesss.j2me.micro3d;
class IntPairs {private final java.util.TreeMap<Integer,Integer> data=new java.util.TreeMap<Integer,Integer>();IntPairs(int n){}void put(int k,int v){data.put(k,v);}int size(){return data.size();}int keyAt(int i){return ((Integer)data.keySet().toArray()[i]).intValue();}int valueAt(int i){return data.get(keyAt(i));}}
