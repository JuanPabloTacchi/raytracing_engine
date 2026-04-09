kernel void sim_life(global unsigned char *in,global unsigned char *out, int N, int M) {
  uint dims = get_work_dim();

  int gindx = get_global_id(0);
  int gindy = get_global_id(1);

  if (gindx < N && gindy < M) {
    
    int x0 = (gindx - 1)%N;
    int x2 = (gindx + 1)%N;
    int y0 = (gindy - 1)%M;
    int y2 = (gindy + 1)%M;
   
    int liveNeighbors = in[x0*M + y0] + in[x0*M+gindy] + in[x0*M+y2] +
                    in[gindx*M + y0] + in[gindx*M+y2] +
                    in[x2*M+y0] + in[x2*M+gindy]+ in[x2*M+y2];
    //barrier(CLK_GLOBAL_MEM_FENCE);
    out[gindx*M+gindy] = (liveNeighbors == 3 || (liveNeighbors == 2 && in[gindx*M+gindy]));
  }
}
