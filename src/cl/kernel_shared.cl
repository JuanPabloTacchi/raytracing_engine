

kernel void sim_life_shared(global unsigned char *in,global unsigned char *out, int N, int M,local unsigned char *shared_mem) {
  uint dims = get_work_dim();
  //global index
  int gindx = get_global_id(0);
  int gindy = get_global_id(1);
  //local index (in the work group)
  int lindx = get_local_id(0);
  int lindy = get_local_id(1);
  //local index in local/shared memory
  int shindx = lindx+1;  //TODO:revisar que sean %N y %M
  int shindy = lindy+1;
  size_t localSizeX = get_local_size(0);
  size_t localSizeY = get_local_size(1);
  size_t sharedSizeX = localSizeX + 2;
  size_t sharedSizeY = localSizeY + 2;
  //const int shared_size = lsize*lsize+4*(lsize+1);


  if (gindx < N && gindy < M) {
    
    int x0 = (gindx - 1)%N;
    int x2 = (gindx + 1)%N;
    int y0 = (gindy - 1)%M;
    int y2 = (gindy + 1)%M;

    shared_mem[shindx*sharedSizeY + shindy] = in[gindx*M + gindy];

    if (lindx == 0) { // threads in the upper horizontal block edge should write the value "above" them into shared memory
        shared_mem[shindy] = in[x0*M + gindy];

        if (lindy == 0) { // upper left corner needs to be copied as well
            shared_mem[0] = in[x0*M + y0];
        }
        else if (lindy == localSizeY-1) { // upper right
            shared_mem[shindy+1] = in[x0*M + y2];
        }
    }
    else if (lindx == localSizeX - 1) { //threads in the lower horizontal block edge should do the same with the value "below" them
        shared_mem[(shindx+1)*sharedSizeY + shindy] = in[x2*M + gindy];

        if (lindy == 0) { //lower left
            shared_mem[(shindx+1)*sharedSizeY] = in[x2*M + y0];//TODO:revisar que esto sta bien 
        }
        else if (lindy == localSizeY-1) { //lower right
            shared_mem[(shindx+1)*sharedSizeY + (shindy+1)] = in[x2*M + y2];
        }

    }

    if (lindy == 0) { // threads in the left vertical block edge should write the value to the left into shared memory
        shared_mem[(shindx)*sharedSizeY] = in[gindx*M + y0];
    }
    else if (lindy == localSizeY - 1) { // threads in the right vertical block edge should write the value to the right into shared memory
        shared_mem[(shindx)*sharedSizeY + (shindy+1)] = in[gindx*M + y2];
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    int liveNeighbors = shared_mem[(shindx-1)*sharedSizeY + (shindy-1)] 
                      + shared_mem[(shindx-1)*sharedSizeY + (shindy)] 
                      + shared_mem[(shindx-1)*sharedSizeY + (shindy+1)] 
                      + shared_mem[(shindx)*sharedSizeY + (shindy-1)]
                      + shared_mem[(shindx)*sharedSizeY + (shindy+1)] 
                      + shared_mem[(shindx+1)*sharedSizeY + (shindy-1)]
                      + shared_mem[(shindx+1)*sharedSizeY + (shindy)]
                      + shared_mem[(shindx+1)*sharedSizeY + (shindy+1)];
    //barrier(CLK_GLOBAL_MEM_FENCE);
    out[gindx*M+gindy] = (liveNeighbors == 3 || (liveNeighbors == 2 && shared_mem[shindx*sharedSizeY + shindy]));
    
  }
}

