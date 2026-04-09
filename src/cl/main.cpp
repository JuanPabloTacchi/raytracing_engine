#include <cstddef>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>



#define CL_HPP_ENABLE_EXCEPTIONS
#include <OpenCL/opencl.hpp>


//Estructura para contar los tiempos de procesamiento
struct Times {
  long create_data = 0;
  long copy_to_host = 0;
  long execution = 0;
  long copy_to_device = 0;
  inline long total() {
    return create_data + copy_to_host + execution + copy_to_device;
  }
};
bool shared_memory = false;
Times t;
cl::Program prog;
cl::CommandQueue queue;

//Settea el program y hace build del kernel
bool init() {
  std::vector<cl::Platform> platforms;
  std::vector<cl::Device> devices;
  //se obtiene la plataforma y device
  cl::Platform::get(&platforms);
  for (auto& p : platforms) {
    p.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (devices.size() > 0) break;
  }
  if (devices.size() == 0) {
    std::cerr << "Not GPU device found" << std::endl;
    return false;
  }

  std::cout << "GPU Used: " << devices.front().getInfo<CL_DEVICE_NAME>()
            << std::endl;

  cl::Context context(devices.front());
  queue = cl::CommandQueue(context, devices.front());
  std::stringstream sourceCode;

  if (shared_memory == true){
    std::ifstream sourceFile("kernel_shared.cl");
    sourceCode << sourceFile.rdbuf();
  }
  else{
    std::ifstream sourceFile("kernel.cl");
    sourceCode << sourceFile.rdbuf();
  }

  prog = cl::Program(context, sourceCode.str(), true);

  return true;
}

void arrayInit(const int N,const int M,unsigned char (&a)[],std::ofstream& out){
  using std::chrono::microseconds;
  
  //Set seed and make sure random works into the unsigned char limits
  //int myseed = 1234;
  //std::default_random_engine rng(myseed);
  //std::uniform_int_distribution<int> rng_dist(0, 255);
  srand(1234);
  // Assign values to host variables
  auto t_start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      a[i*M + j] = rand() % 2;
    }
  }
  auto t_end = std::chrono::high_resolution_clock::now();
  t.create_data =
      std::chrono::duration_cast<microseconds>(t_end - t_start).count();
  out << "\n";
  for (int i = 0; i < N; i++){
    for (int j = 0; j < M; j++){
      int printerAux = a[i*M + j];
      out << printerAux;
      if (j != M-1){
        out << ",";
      }
      else{
        out << "\n";
      }
    }
  }
  out << "\n";

}


 void arrayLoad(const int N,const int M,unsigned char (&a)[],std::ofstream& out,std::string archive_in){ 
  using std::chrono::microseconds;
  auto t_start = std::chrono::high_resolution_clock::now();


  std::ifstream archivo (archive_in);
  if (!archivo.is_open()){
      throw std::logic_error("Main.cl: Error abriendo archivo.");
  }

  std::string valor;
  std::vector<double> numeros;
  int i = 0;
  while (std::getline(archivo, valor,',')) {
    if(valor != "\n"){
      a[i] = static_cast<unsigned char>(std::stoi(valor));
      i++;
    }
  }
  archivo.close();
  auto t_end = std::chrono::high_resolution_clock::now();
  t.create_data =
      std::chrono::duration_cast<microseconds>(t_end - t_start).count();
  out << "\n";
  for (int i = 0; i < N; i++){
      for (int j = 0; j < M; j++){
        int printerAux = a[i*M + j];
        out << printerAux;
        if (j != M-1){
          out << ",";
        }
        else{
          out << "\n";
        }
      }
    }
    out << "\n";
  }

//Simula 1 epoca del juego en GPU
//N,M Tamaño de la grilla
//globalsize tamaño del problema
//localsize tamaño de los work-groups
//out archivo abierto para enviar los resultados
bool simulate(const int N,const int M, int localSize, std::ofstream& out,unsigned char (&a)[]) {
  using std::chrono::microseconds;
  const std::size_t size = sizeof(unsigned char) * N * M;
  

  // Create the memory buffers
  cl::Buffer inBuff(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, size);
  cl::Buffer outBuff(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, size);
  //Set seed and make sure random works into the unsigned char limits
  //int myseed = 1234;
  //std::default_random_engine rng(myseed);
  //std::uniform_int_distribution<int> rng_dist(0, 255);

 

  // Copy values from host variables to device
  auto t_start = std::chrono::high_resolution_clock::now();
  // usar CL_FALSE para hacerlo asíncrono
  queue.enqueueWriteBuffer(inBuff, CL_TRUE, 0, size, a);

  auto t_end = std::chrono::high_resolution_clock::now();
  t.copy_to_device =
      std::chrono::duration_cast<microseconds>(t_end - t_start).count() + t.copy_to_device;

  // Make kernel
  cl::Kernel kernel(prog, "sim_life");

  // Set the kernel arguments
  kernel.setArg(0, inBuff);
  kernel.setArg(1, outBuff);
  kernel.setArg(2, N);
  kernel.setArg(3, M);

  // Execute the function on the device 
  size_t globalSizeN = ceil(N/localSize)*localSize;
  size_t globalSizeM = ceil(M/localSize)*localSize;
  cl::NDRange globalSize2D(globalSizeN,globalSizeM);
  cl::NDRange localsize2D(localSize,localSize);

  t_start = std::chrono::high_resolution_clock::now();
  cl::Event event;
  queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize2D, localsize2D,nullptr,&event);
  //queue.finish();
  event.wait();
  t_end = std::chrono::high_resolution_clock::now();
  t.execution =
      std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start)
          .count() + t.execution;

  // Copy the output variable from device to host
  t_start = std::chrono::high_resolution_clock::now();
  queue.enqueueReadBuffer(outBuff, CL_TRUE, 0, size, a);
  t_end = std::chrono::high_resolution_clock::now();
  t.copy_to_host =
      std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start)
          .count() + t.copy_to_host;

  // Print the result

  for (int i = 0; i < N; i++){
    for (int j = 0; j < M; j++){
      int printerAux = a[i*M + j];
      out  << printerAux;
      if (j != M-1){
        out << ",";
      }
      else{
        out << "\n";
      }
    }
  }
  out << "\n";
  

  std::cout << "Time to create data: " << t.create_data << " microseconds\n";
  std::cout << "Time to copy data to device: " << t.copy_to_device
            << " microseconds\n";
  std::cout << "Time to execute kernel: " << t.execution << " microseconds\n";
  std::cout << "Time to copy data to host: " << t.copy_to_host
            << " microseconds\n";
  std::cout << "Time to execute the whole program: " << t.total()
            << " microseconds\n";

  
  return true;
}

//simula con memoria compartida
bool simulate_shared(const int N,const int M, int localSize, std::ofstream& out,unsigned char (&a)[]) {
  using std::chrono::microseconds;
  const std::size_t size = sizeof(unsigned char) * N * M;
  

  // Create the memory buffers
  cl::Buffer inBuff(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, size);
  cl::Buffer outBuff(queue.getInfo<CL_QUEUE_CONTEXT>(), CL_MEM_READ_WRITE, size);
  //Set seed and make sure random works into the unsigned char limits
  //int myseed = 1234;
  //std::default_random_engine rng(myseed);
  //std::uniform_int_distribution<int> rng_dist(0, 255);

 

  // Copy values from host variables to device
  auto t_start = std::chrono::high_resolution_clock::now();
  // usar CL_FALSE para hacerlo asíncrono
  queue.enqueueWriteBuffer(inBuff, CL_TRUE, 0, size, a);

  auto t_end = std::chrono::high_resolution_clock::now();
  t.copy_to_device =
      std::chrono::duration_cast<microseconds>(t_end - t_start).count() + t.copy_to_device;

 
  // Make kernel
  cl::Kernel kernel(prog, "sim_life_shared");

  // Set the kernel arguments
  kernel.setArg(0, inBuff);
  kernel.setArg(1, outBuff);
  kernel.setArg(2, N);
  kernel.setArg(3, M);
  cl::size_type shared_mem_size = ((localSize+2)*(localSize+2)) * sizeof(unsigned char);
  kernel.setArg(4, shared_mem_size, NULL);
  
  // Execute the function on the device 
  size_t globalSizeN = ceil(N/localSize)*localSize;
  size_t globalSizeM = ceil(M/localSize)*localSize;
  cl::NDRange globalSize2D(globalSizeN,globalSizeM);
  cl::NDRange localsize2D(localSize,localSize);

  t_start = std::chrono::high_resolution_clock::now();
  cl::Event event;
  queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize2D, localsize2D,nullptr,&event);
  //queue.finish();
  event.wait();
  t_end = std::chrono::high_resolution_clock::now();
  t.execution =
      std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start)
          .count() + t.execution;

  // Copy the output variable from device to host
  t_start = std::chrono::high_resolution_clock::now();
  //queue.enqueueReadBuffer(outBuff, CL_TRUE, 0, size, a);
  t_end = std::chrono::high_resolution_clock::now();
  t.copy_to_host =
      std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start)
          .count() + t.copy_to_host;

  // Print the result

  for (int i = 0; i < N; i++){
    for (int j = 0; j < M; j++){
      int printerAux = a[i*M + j];
      out  << printerAux;
      if (j != M-1){
        out << ",";
      }
      else{
        out << "\n";
      }
    }
  }
  out << "\n";
  

  std::cout << "Time to create data: " << t.create_data << " microseconds\n";
  std::cout << "Time to copy data to device: " << t.copy_to_device
            << " microseconds\n";
  std::cout << "Time to execute kernel: " << t.execution << " microseconds\n";
  std::cout << "Time to copy data to host: " << t.copy_to_host
            << " microseconds\n";
  std::cout << "Time to execute the whole program: " << t.total()
            << " microseconds\n";

  
  return true;
}


int main(int argc, char* argv[]) {

  if (argc != 7 && argc != 8) {
    std::cerr << "Uso: " << argv[0]
              << " <array size N> <array size M> <local size> <t-iterations> <shared memory> <output filename> <input filename>"
              << std::endl;
    return 2;
  }
  const int n = std::stoi(argv[1]);
  const int m = std::stoi(argv[2]);
  int ls = std::stoi(argv[3]);
  int T = std::stoi(argv[4]);
  shared_memory = bool(std::stoi(argv[5]));
  std::cout << shared_memory << "\n";

  if (!init()){
    std::cerr << "Error inicializando OpenCL";
    return 1;
  }
  

  

  //Abre el archivo
  std::ofstream out;
  out.open(argv[6], std::ios::app | std::ios::out);
  if (!out.is_open()) {
    std::cerr << "Error while opening file: '" << argv[6] << "'" << std::endl;
    return 4;
  }

  // params
  out << n << "," << ls << ",";


  //Simulation 
  unsigned char a[n*m];
  //a load
  if (argv[7] != NULL){
    
    arrayLoad(n,m,a,out,argv[7]);
  }
  else{
    arrayInit(n,m,a,out);
  }
  
  
  


  for (int i = 0; i < T;i++){
    //Simula 1 epoca
    if (shared_memory == true){
        if (!simulate_shared(n, m, ls, out, a)) {
          std::cerr << "CL: Error while executing the simulation" << std::endl;
          return 3;
        }
    }
    else{
        if (!simulate(n, m, ls, out, a)) {
          std::cerr << "CL: Error while executing the simulation" << std::endl;
          return 3;
        }
    }



  }
  

  
  // times
  out << t.create_data << "," << t.copy_to_device << "," << t.execution << ","
      << t.copy_to_host << "," << t.total() << "\n";

  std::cout << "Data written to " << argv[6] << std::endl;
  return 0;
}
