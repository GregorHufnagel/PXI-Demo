#include <iostream>

#include "PXIe4190.h"

#ifdef basic_version
int main()
{
    LCR_AdvancedSequenceWithTriggerv33();
}
#endif

#ifdef first_stage
int main() {
    create_lcr("LCR1");
    create_smu("SMU1");

    while (1) {
        if (trigger_lcr_seq("LCR1")) {
            break;
        };
        if (trigger_smu_seq("SMU1")) {
            break;
        };
    }
    return 0;
}
#endif

#ifdef second_stage
int main() {
    create_lcr("LCR1");
    create_smu("SMU1");
    create_lcr("LCR2");
    create_smu("SMU2");

    while (1) {
        if (trigger_lcr_seq("LCR2")) {
            break;
        };
        if (trigger_smu_seq("SMU1")) {
            break;
        };
        if (trigger_lcr_seq("LCR1")) {
            break;
        };
        if (trigger_smu_seq("SMU2")) {
            break;
        };
    }
    return 0;
}
#endif


#ifdef combi_stage
int main() {
    init_session();
    perform_compensation();
    create_lcr("LCR1");

    while (1) {
        if (trigger_lcr_seq("LCR1")) {
            break;
        };
        //if (trigger_smu_seq("SMU1")) {
        //    break;
        //};
    }
    return 0;
}
#endif