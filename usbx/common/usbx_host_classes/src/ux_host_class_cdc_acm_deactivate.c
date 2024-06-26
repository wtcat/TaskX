/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** USBX Component                                                        */ 
/**                                                                       */
/**   CDC ACM Class                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"
#include "ux_host_class_cdc_acm.h"
#include "ux_host_stack.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_host_class_cdc_acm_deactivate                   PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function is called when this instance of the cdc_acm has been  */
/*    removed from the bus either directly or indirectly. The bulk in\out */ 
/*    and optional interrupt pipes will be destroyed and the instance     */ 
/*    removed.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    command                               CDC ACM class command pointer */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_host_stack_class_instance_destroy Destroy the class instance    */ 
/*    _ux_host_stack_endpoint_transfer_abort Abort endpoint transfer      */ 
/*    _ux_utility_memory_free               Free memory block             */ 
/*    _ux_host_semaphore_get                Get protection semaphore      */ 
/*    _ux_host_semaphore_delete             Delete protection semaphore   */ 
/*    _ux_utility_thread_schedule_other     Schedule other threads        */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _ux_host_class_cdc_acm_entry          Entry of cdc_acm class        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Chaoqiong Xiao           Modified comment(s),          */
/*                                            added standalone support,   */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _ux_host_class_cdc_acm_deactivate(UX_HOST_CLASS_COMMAND *command)
{

UX_HOST_CLASS_CDC_ACM       *cdc_acm;
UX_TRANSFER                 *transfer_request;
#if !defined(UX_HOST_STANDALONE)
UINT                        status;
#else
UX_HOST_CLASS_CDC_ACM       *cdc_acm_inst;
#endif

    /* Get the instance for this class.  */
    cdc_acm =  (UX_HOST_CLASS_CDC_ACM *) command -> ux_host_class_command_instance;

    /* The cdc_acm is being shut down.  */
    cdc_acm -> ux_host_class_cdc_acm_state =  UX_HOST_CLASS_INSTANCE_SHUTDOWN;

#if !defined(UX_HOST_STANDALONE)

    /* Protect thread reentry to this instance.  */
    status =  _ux_host_semaphore_get(&cdc_acm -> ux_host_class_cdc_acm_semaphore, UX_WAIT_FOREVER);
    if (status != UX_SUCCESS)

        /* Return error.  */
        return(status);
#endif

    /* If we have the Control Class, we only unmount the interrupt endpoint if it is active.  */
    if (cdc_acm -> ux_host_class_cdc_acm_interface -> ux_interface_descriptor.bInterfaceClass == UX_HOST_CLASS_CDC_CONTROL_CLASS)
    {

        /* If the interrupt endpoint is defined, clean any pending transfer.  */
        if (cdc_acm -> ux_host_class_cdc_acm_interrupt_endpoint != UX_NULL)
        {    
            transfer_request =  &cdc_acm -> ux_host_class_cdc_acm_interrupt_endpoint -> ux_endpoint_transfer_request;
            
            /* And abort any transfer.  */
            _ux_host_stack_endpoint_transfer_abort(cdc_acm -> ux_host_class_cdc_acm_interrupt_endpoint);

            /* Free data buffer for the interrupt transfer.  */
            _ux_utility_memory_free(transfer_request -> ux_transfer_request_data_pointer);
        }

#if defined(UX_HOST_STANDALONE)

        /* Linked from data instance must be removed then.  */
        /* We scan CDC ACM instances to find the master instance.  */
        /* Get first instance linked to the class.  */
        cdc_acm_inst = (UX_HOST_CLASS_CDC_ACM *) cdc_acm ->
                    ux_host_class_cdc_acm_class -> ux_host_class_first_instance;

        /* Scan all instances.  */
        while(cdc_acm_inst)
        {

            /* If this data interface is linking to the control instance, unlink it.  */
            if (cdc_acm_inst -> ux_host_class_cdc_acm_control == cdc_acm)
                cdc_acm_inst -> ux_host_class_cdc_acm_control = UX_NULL;

            /* Next instance.  */
            cdc_acm_inst = cdc_acm_inst -> ux_host_class_cdc_acm_next_instance;
        }
#endif
    }
    else
    {
    
        /* We come to this point when the device has been extracted and this is the data class. 
           So there may have been a transaction being scheduled. 
           We make sure the transaction has been completed by the controller driver.
           When the device is extracted, the controller tries multiple times the transaction and retires it
           with a DEVICE_NOT_RESPONDING error code.  
           
           First we take care of endpoint IN.  */
        transfer_request =  &cdc_acm -> ux_host_class_cdc_acm_bulk_in_endpoint -> ux_endpoint_transfer_request;
        if (transfer_request -> ux_transfer_request_completion_code == UX_TRANSFER_STATUS_PENDING)

            /* We need to abort transactions on the bulk In pipe.  */
            _ux_host_stack_endpoint_transfer_abort(cdc_acm -> ux_host_class_cdc_acm_bulk_in_endpoint);
    
        /* Then endpoint OUT.  */       
        transfer_request =  &cdc_acm -> ux_host_class_cdc_acm_bulk_out_endpoint -> ux_endpoint_transfer_request;
        if (transfer_request -> ux_transfer_request_completion_code == UX_TRANSFER_STATUS_PENDING)
           
            /* We need to abort transactions on the bulk Out pipe. */
            _ux_host_stack_endpoint_transfer_abort(cdc_acm -> ux_host_class_cdc_acm_bulk_out_endpoint);
    
    }

#if !defined(UX_HOST_STANDALONE)

    /* The enumeration thread needs to sleep a while to allow the application or the class that may be using
       endpoints to exit properly.  */
    _ux_host_thread_schedule_other(UX_THREAD_PRIORITY_ENUM); 
#else

    /* If there is allocated resource, free it.  */
    if (cdc_acm -> ux_host_class_cdc_acm_allocated)
    {
        _ux_utility_memory_free(cdc_acm -> ux_host_class_cdc_acm_allocated);
    }
#endif

    /* Destroy the instance.  */
    _ux_host_stack_class_instance_destroy(cdc_acm -> ux_host_class_cdc_acm_class, (VOID *) cdc_acm);

#if !defined(UX_HOST_STANDALONE)

    /* Destroy the semaphore.  */
    _ux_host_semaphore_delete(&cdc_acm -> ux_host_class_cdc_acm_semaphore);
#endif

    /* Before we free the device resources, we need to inform the application
        that the device is removed.  */
    if (_ux_system_host -> ux_system_host_change_function != UX_NULL)
    {
        
        /* Inform the application the device is removed.  */
        _ux_system_host -> ux_system_host_change_function(UX_DEVICE_REMOVAL, cdc_acm -> ux_host_class_cdc_acm_class, (VOID *) cdc_acm);
    }

    /* Free the cdc_acm instance memory.  */
    _ux_utility_memory_free(cdc_acm);

    /* If trace is enabled, insert this event into the trace buffer.  */
    UX_TRACE_IN_LINE_INSERT(UX_TRACE_HOST_CLASS_CDC_ACM_DEACTIVATE, cdc_acm, 0, 0, 0, UX_TRACE_HOST_CLASS_EVENTS, 0, 0)

    /* If trace is enabled, register this object.  */
    UX_TRACE_OBJECT_UNREGISTER(cdc_acm);

    /* Return successful status.  */
    return(UX_SUCCESS);         
}

