#include "types.h"
#include "user.h"

int
main(void)
{
  int count, total_weight, period;

  if(gettreeinfo(&count, &total_weight, &period) < 0){
    printf(1, "Error: gettreeinfo failed\n");
    exit();
  }

  printf(1, "Tree Info - Count: %d, Total Weight: %d, Period: %d\n", count, total_weight, period);

  // since no processes are runnable, the tree should be empty
  if(count == 0){
    printf(1, "Test Passed: Tree initialized properly with the init process\n");
  } else {
    printf(1, "Test Failed: Tree count expected to be 0, got %d\n", count);
  }
  if(total_weight == 0){
    printf(1, "Test Passed: Total weight initialized properly\n");
  } else {
    printf(1, "Test Failed: Total weight expected to be 0, got %d\n", total_weight);
  }
  if(period == 32){
    printf(1, "Test Passed: Period initialized properly\n");
  } else {
    printf(1, "Test Failed: Period expected to be 32, got %d\n", period);
  }

  printf(1, "Test completed\n");
  exit();
}
