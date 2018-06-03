SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_CFLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g
else
        SGX_COMMON_CFLAGS += -O2
endif

######## ImgProcessFileApp Settings ########

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

NASM := nasm

App_Cpp_Files := ImgProcessFileApp/ImgProcessFileApp.cpp
App_Include_Paths := -IInclude -IApp -I$(SGX_SDK)/include -IImgProcessFileApp/Benchmark

App_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(App_Include_Paths)

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Cpp_Flags := $(App_C_Flags) -std=c++11
App_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread 

ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif

App_Cpp_Objects := $(App_Cpp_Files:.cpp=.o)

App_Name := sgxImgProcessFile

######## ImgProcessEnclave Settings ########

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto

ImgProcessEnclave_Cpp_Files := ImgProcessEnclave/ImgProcessEnclave.cpp
ImgProcessEnclave_Include_Paths := -IInclude -IImgProcessEnclave -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/stlport

ImgProcessEnclave_C_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(ImgProcessEnclave_Include_Paths)
ImgProcessEnclave_Cpp_Flags := $(ImgProcessEnclave_C_Flags) -std=c++03 -nostdinc++
ImgProcessEnclave_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=ImgProcessEnclave/ImgProcessEnclave.lds

ImgProcessEnclave_Cpp_Objects := $(ImgProcessEnclave_Cpp_Files:.cpp=.o)

ImgProcessEnclave_Name := ImgProcessEnclave.so
Signed_ImgProcessEnclave_Name := ImgProcessEnclave.signed.so
ImgProcessEnclave_Config_File := ImgProcessEnclave/ImgProcessEnclave.config.xml

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all run

ifeq ($(Build_Mode), HW_RELEASE)
all: $(App_Name) $(ImgProcessEnclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(ImgProcessEnclave_Name) first with your signing key before you run the $(App_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(ImgProcessEnclave_Name) -out <$(Signed_ImgProcessEnclave_Name)> -config $(ImgProcessEnclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool. See User's Guide for more details."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."
else
all: $(App_Name) $(Signed_ImgProcessEnclave_Name)
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/$(App_Name)
	@echo "RUN  =>  $(App_Name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

######## ImgProcessFileApp Objects ########

ImgProcessFileApp/ImgProcessEnclave_u.c: $(SGX_EDGER8R) ImgProcessEnclave/ImgProcessEnclave.edl
	@cd ImgProcessFileApp && $(SGX_EDGER8R) --untrusted ../ImgProcessEnclave/ImgProcessEnclave.edl --search-path ../ImgProcessEnclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

ImgProcessFileApp/ImgProcessEnclave_u.o: ImgProcessFileApp/ImgProcessEnclave_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

ImgProcessFileApp/%.o: ImgProcessFileApp/%.cpp
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

ImgProcessFileApp/Benchmark/cpuidc64.o: ImgProcessFileApp/Benchmark/cpuidc64.c
	@$(CC) -m64 $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

ImgProcessFileApp/Benchmark/cpuida64.o: ImgProcessFileApp/Benchmark/cpuida64.asm
	@$(NASM) -f elf64 $<
	@echo "NASM  <=  $<"

$(App_Name): ImgProcessFileApp/Benchmark/cpuidc64.o ImgProcessFileApp/Benchmark/cpuida64.o ImgProcessFileApp/ImgProcessEnclave_u.o $(App_Cpp_Objects)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"


######## ImgProcessEnclave Objects ########

ImgProcessEnclave/ImgProcessEnclave_t.c: $(SGX_EDGER8R) ImgProcessEnclave/ImgProcessEnclave.edl
	@cd ImgProcessEnclave && $(SGX_EDGER8R) --trusted ../ImgProcessEnclave/ImgProcessEnclave.edl --search-path ../ImgProcessEnclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

ImgProcessEnclave/ImgProcessEnclave_t.o: ImgProcessEnclave/ImgProcessEnclave_t.c
	@$(CC) $(ImgProcessEnclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

ImgProcessEnclave/%.o: ImgProcessEnclave/%.cpp
	@$(CXX) $(ImgProcessEnclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(ImgProcessEnclave_Name): ImgProcessEnclave/ImgProcessEnclave_t.o $(ImgProcessEnclave_Cpp_Objects)
	@$(CXX) $^ -o $@ $(ImgProcessEnclave_Link_Flags)
	@echo "LINK =>  $@"

$(Signed_ImgProcessEnclave_Name): $(ImgProcessEnclave_Name)
	@$(SGX_ENCLAVE_SIGNER) sign -key ImgProcessEnclave/ImgProcessEnclave_private.pem -enclave $(ImgProcessEnclave_Name) -out $@ -config $(ImgProcessEnclave_Config_File)
	@echo "SIGN =>  $@"

.PHONY: clean

clean:
	@rm -f $(App_Name) $(ImgProcessEnclave_Name) $(Signed_ImgProcessEnclave_Name) $(App_Cpp_Objects) ImgProcessFileApp/Benchmark/*.o ImgProcessFileApp/ImgProcessEnclave_u.* $(ImgProcessEnclave_Cpp_Objects) ImgProcessEnclave/ImgProcessEnclave_t.*
