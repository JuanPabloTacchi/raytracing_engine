kernel void sim_life_test(global unsigned char *in,global unsigned char *out, int N, int M, local unsigned char *shared_mem) {
  uint dims = get_work_dim();
  //global index
  int gindx = get_global_id(0);
  int gindy = get_global_id(1);
  //local index (in the work group)

 
  
 
    //barrier(CLK_GLOBAL_MEM_FENCE);
    
  
}