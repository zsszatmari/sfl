#include <signal.h>
#include <execinfo.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "json.h"
#include "FakePlaybackData.h"
#include "stdplus.h"
#include MEMORY_H

using namespace Base;
using namespace Gear;


/* Resolve symbol name and source location given the path to the executable 
   and an address */
int addr2line(char const * const program_name, void const * const addr)
{
  char addr2line_cmd[512] = {0};
  //printf("mu\n"); fflush(NULL);

  /* have addr2line map the address to the relent line in the code */
  #ifdef __APPLE__
    /* apple does things differently... */
    sprintf(addr2line_cmd,"xcrun atos -o %.256s %p", program_name, addr); 
    //printf("xcrun atos -o %.256s %p\n", program_name, addr);
  #else
    sprintf(addr2line_cmd,"addr2line -f -p -e %.256s %p", program_name, addr); 
  #endif
 
  /* This will print a nicely formatted string specifying the
     function and source line of the address */
  return system(addr2line_cmd);
}

#define MAX_STACK_FRAMES 64
static char const * icky_global_program_name;
static void *stack_traces[MAX_STACK_FRAMES];
static bool keepRunning = false; 
void posix_print_stack_trace()
{
  int i, trace_size = 0;
  char **messages = (char **)NULL;
 
  trace_size = backtrace(stack_traces, MAX_STACK_FRAMES);
  messages = backtrace_symbols(stack_traces, trace_size);
 
  /* skip the first couple stack frames (as they are this function and
     our handler) and also skip the last frame as it's (always?) junk. */
  // for (i = 3; i < (trace_size - 1); ++i)
  // we'll use this for now so you can see what's going on
  for (i = 0; i < trace_size; ++i)
  {
    if (addr2line(icky_global_program_name, stack_traces[i]) != 0)
    {
      printf("  error determining line # for: %s\n", messages[i]);
    }
 
  }
  if (messages) { free(messages); } 
}

void myterminate(int sig) 
{
	keepRunning = true;
	std::cerr << "got signal " << sig << "\n";
	posix_print_stack_trace();
	_Exit(1);
}


int main(int argc, char** argv) 
{
	icky_global_program_name = argv[0];
	//std::set_terminate(myterminate);
	signal(SIGABRT, myterminate);
	signal(SIGSEGV, myterminate);
	signal(SIGBUS, myterminate);


	// The following line must be executed to initialize Google Mock
	// (and Google Test) before running the tests.
	::testing::InitGoogleMock(&argc, argv);

	::testing::DefaultValue<const MEMORY_NS::shared_ptr<Json::Value>>::Set(MEMORY_NS::shared_ptr<Json::Value>(new Json::Value()));
	::testing::DefaultValue<MEMORY_NS::shared_ptr<IPlaybackData>>::Set(MEMORY_NS::shared_ptr<IPlaybackData>(new FakePlaybackData()));
	
	//::testing::GTEST_FLAG(filter) = "StringCompare.*";

	return RUN_ALL_TESTS();
}
