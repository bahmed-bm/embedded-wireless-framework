/************************************************************************//**
 * @file
 * @version Preview
 * @copyright Copyright (c) Microsoft Corporation. All rights reserved.
 * SPDX-License-Identifier: MIT
 * @brief The Embedded Wireless Framework adapter NetX Duo test example.
 ****************************************************************************/

#include "ewf_allocator_threadx.h"
#include "ewf_interface_win32_com.h"
#include "ewf_adapter_renesas_ryz024a.h"
#include "ewf_lib.h"
#include "ewf_example.config.h"

/* Define the thread for running Azure SDK on ThreadX (THREADX IoT Platform).  */
#ifndef SAMPLE_STACK_SIZE
#define SAMPLE_STACK_SIZE      (2048)
#endif /* SAMPLE_STACK_SIZE  */

#ifndef SAMPLE_THREAD_PRIORITY
#define SAMPLE_THREAD_PRIORITY (4)
#endif /* SAMPLE_THREAD_PRIORITY  */

/* The thread control block  */
static TX_THREAD  sample_thread;

/* Define the thread stack.  */
static ULONG sample_thread_stack[SAMPLE_STACK_SIZE / sizeof(ULONG)];

/* The thread entry point.  */
void sample_thread_entry(ULONG parameter);

/* The EWF NetX Duo PPP test.  */
ewf_result ewf_example_netx_duo_ppp_test(ewf_adapter* adapter_ptr);

/**
 *  @brief The application entry point, initialize the hardware and start ThreadX
 */
int main(int argc, char** argv)
{
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();

    /* We never get here, but we keep the compiler happy.  */
    return 0;
}

/**
 *  @brief Define what the initial ThreadX system looks like.
 */
void tx_application_define(void* first_unused_memory)
{
    UINT  status;

    /* Create sample thread. */
    status = tx_thread_create(&sample_thread, "Sample Thread",
        sample_thread_entry, 0,
        sample_thread_stack, SAMPLE_STACK_SIZE,
        SAMPLE_THREAD_PRIORITY, SAMPLE_THREAD_PRIORITY,
        TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Check for thread creation error.  */
    if (status)
    {
        exit(status);
    }
}

/**
 *  @brief The sample thread entry point
 */
void sample_thread_entry(ULONG parameter)
{
    ewf_result result;

    ewf_allocator* message_allocator_ptr = NULL;
    ewf_interface* interface_ptr = NULL;
    ewf_adapter* adapter_ptr = NULL;

    EWF_ALLOCATOR_THREADX_STATIC_DECLARE(message_allocator_ptr, message_allocator,
        EWF_CONFIG_MESSAGE_ALLOCATOR_BLOCK_COUNT,
        EWF_CONFIG_MESSAGE_ALLOCATOR_BLOCK_SIZE);
    EWF_INTERFACE_WIN32_COM_STATIC_DECLARE(interface_ptr, com_port,
        EWF_CONFIG_INTERFACE_WIN32_COM_PORT_FILE_NAME,
        EWF_CONFIG_INTERFACE_WIN32_COM_PORT_BAUD_RATE,
        EWF_CONFIG_INTERFACE_WIN32_COM_PORT_BYTE_SIZE,
        EWF_CONFIG_INTERFACE_WIN32_COM_PORT_PARITY,
        EWF_CONFIG_INTERFACE_WIN32_COM_PORT_STOP_BITS);
    EWF_ADAPTER_RENESAS_RYZ024A_STATIC_DECLARE(adapter_ptr, renesas_ryz024a, message_allocator_ptr, NULL, interface_ptr);

    // Start the adapter
    if (ewf_result_failed(result = ewf_adapter_start(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to start the adapter, ewf_result %d.\n", result);
        exit(result);
    }
    else
    {
        EWF_LOG("Adapter Start.\n");
    }

    // Set the ME functionality to minimum to clear out any previous connections
    if (ewf_result_failed(result = ewf_adapter_modem_functionality_set(adapter_ptr, EWF_ADAPTER_MODEM_FUNCTIONALITY_MINIMUM)))
    {
        EWF_LOG("[Warning][Failed to the ME functionality]\n");
    }
    else
    {
        EWF_LOG("Modem functionality set to EWF_ADAPTER_MODEM_FUNCTIONALITY_MINIMUM\n");
    }

    // Set the APN
    if (ewf_result_failed(result = ewf_adapter_modem_pdp_apn_set(adapter_ptr, EWF_CONFIG_CONTEXT_ID, EWF_ADAPTER_MODEM_PDP_TYPE_IP, EWF_CONFIG_SIM_APN)))
    {
        EWF_LOG_ERROR("Failed to the set APN, ewf_result %d.\n", result);
        exit(result);
    }
    else
    {
        EWF_LOG("APN Set to %s\n", EWF_CONFIG_SIM_APN);
    }

    // Set the ME functionality
    if (ewf_result_failed(result = ewf_adapter_modem_functionality_set(adapter_ptr, EWF_ADAPTER_MODEM_FUNCTIONALITY_FULL)))
    {
        EWF_LOG_ERROR("Failed to the ME functionality, ewf_result %d.\n", result);
        return;
    }
    else
    {
        EWF_LOG("Modem functionality set to EWF_ADAPTER_MODEM_FUNCTIONALITY_FULL\n");
    }


    /* Wait time for modem to be ready after modem is registered to network */
    ewf_platform_sleep(200);

    // Set the SIM PIN
    if (ewf_result_failed(result = ewf_adapter_modem_sim_pin_enter(adapter_ptr, EWF_CONFIG_SIM_PIN)))
    {
        EWF_LOG_ERROR("Failed to the SIM PIN, ewf_result %d.\n", result);
        exit(result);
    }
    else
    {
        EWF_LOG("SIM PIN Set.\n");
    }

    if (ewf_result_failed(result = ewf_adapter_modem_network_registration_check(adapter_ptr, EWF_ADAPTER_MODEM_CMD_QUERY_EPS_NETWORK_REG, 1000)))
    {
        EWF_LOG("[ERROR][Failed to register to network.]\n");
        return;
    }
	else
	{
		EWF_LOG("Registered to network.\n");
	}

    // Call the NetX Duo test example
    if (ewf_result_failed(result = ewf_example_netx_duo_ppp_test(adapter_ptr)))
    {
        EWF_LOG_ERROR("The NetX Duo test example failed, ewf_result %d.\n", result);
        exit(result);
    }
    	else
	{
		EWF_LOG("The NetX Duo test example passed.\n");
	}
    
    EWF_LOG("\nDone!\n");

    /* Stay here forever.  */
    while (1)
    {
        EWF_LOG(".");
        ewf_platform_sleep(EWF_PLATFORM_TICKS_PER_SECOND);
    }
}

/* Define size of buffer to display client's local time. */
#define BUFSIZE 50

/* Define the client thread.  */
void    demo_client_thread_entry(ULONG info)
{

    UINT   status;
    UINT   spin;
    UINT   server_status;
    ULONG  base_seconds;
    ULONG  base_fraction;
    ULONG  seconds, milliseconds, microseconds, fraction;
    UINT   wait = 0;
    UINT   error_counter = 0;
    ULONG  events = 0;
#ifdef USE_IPV6
    NXD_ADDRESS sntp_server_address;
    NXD_ADDRESS client_ip_address;
#endif

    NX_PARAMETER_NOT_USED(info);

    /* Give other threads (IP instance) initialize first. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#ifdef USE_IPV6
    /* Set up IPv6 services. */
    status = nxd_ipv6_enable(&client_ip);

    status += nxd_icmp_enable(&client_ip);

    if (status != NX_SUCCESS)
        return;

    client_ip_address.nxd_ip_address.v6[0] = 0x20010db8;
    client_ip_address.nxd_ip_address.v6[1] = 0x0000f101;
    client_ip_address.nxd_ip_address.v6[2] = 0x0;
    client_ip_address.nxd_ip_address.v6[3] = 0x101;
    client_ip_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Set the IPv6 server address. */
    sntp_server_address.nxd_ip_address.v6[0] = 0x20010db8;
    sntp_server_address.nxd_ip_address.v6[1] = 0x0000f101;
    sntp_server_address.nxd_ip_address.v6[2] = 0x0;
    sntp_server_address.nxd_ip_address.v6[3] = 0x00000106;
    sntp_server_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Establish the link local address for the host. The RAM driver creates
       a virtual MAC address. */
    status = nxd_ipv6_address_set(&client_ip, iface_index, NX_NULL, 10, NULL);

    /* Check for link local address set error.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Set the host global IP address. We are assuming a 64
      bit prefix here but this can be any value (< 128). */
    status = nxd_ipv6_address_set(&client_ip, iface_index,
        &client_ip_address,
        prefix, &address_index);

    /* Check for global address set error.  */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Wait while NetX Duo validates the global and link local addresses. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);

#endif

    /* Setup time update callback function. */
    nx_sntp_client_set_time_update_notify(&demo_sntp_client,
        time_update_callback);

    /* Set up client time updates depending on mode. */
#ifdef USE_UNICAST

    /* Initialize the Client for unicast mode to
       poll the SNTP server once an hour. */
#ifdef USE_IPV6
       /* Use the duo service to set up the Client and set the IPv6 SNTP server.
          Note: this can take either an IPv4 or IPv6 address. */
    status = nxd_sntp_client_initialize_unicast(&demo_sntp_client,
        &sntp_server_address);
#else
       /* Use the IPv4 service to set up the Client and set the IPv4 SNTP server. */
    status = nx_sntp_client_initialize_unicast(&demo_sntp_client,
        SERVER_IP_ADDRESS);
#endif  /* USE_IPV6 */


#else   /* Broadcast mode */

/* Initialize the Client for broadcast mode, no roundtrip calculation
   required and a broadcast SNTP service. */
#ifdef USE_IPV6
   /* Use the duo service to initialize the Client
      and set IPv6 SNTP all hosts multicast address.
      (Note: This can take either an IPv4 or IPv6 address.)*/
    status = nxd_sntp_client_initialize_broadcast(&demo_sntp_client,
        &sntp_server_address,
        NX_NULL);
#else

   /* Use the IPv4 service to initialize the Client and set
      IPv4 SNTP broadcast address. */
    status = nx_sntp_client_initialize_broadcast(&demo_sntp_client,
        NX_NULL,
        SERVER_IP_ADDRESS);
#endif  /* USE_IPV6 */
#endif  /* USE_UNICAST */

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Set the base time which is approximately the number of seconds since
       the turn of the last century. If this is not available in SNTP format,
       the nx_sntp_client_utility_add_msecs_to_ntp_time service can convert
       milliseconds to fraction.  For how to compute NTP seconds from real
   time, read the NetX SNTP User Guide. Otherwise set the base time to
   zero and set NX_SNTP_CLIENT_IGNORE_MAX_ADJUST_STARTUP to NX_TRUE for
       the SNTP CLient to accept the first time update without applying a
       minimum or maximum adjustment parameters
      (NX_SNTP_CLIENT_MIN_TIME_ADJUSTMENT and
       NX_SNTP_CLIENT_MAX_TIME_ADJUSTMENT). */

    base_seconds = 0xd2c96b90;  /* Jan 24, 2012 UTC */
    base_fraction = 0xa132db1e;

    /* Apply to the SNTP Client local time.  */
    status = nx_sntp_client_set_local_time(&demo_sntp_client, base_seconds,
        base_fraction);

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
        return;
    }

    /* Run whichever service the client is configured for. */
#ifdef USE_UNICAST
    status = nx_sntp_client_run_unicast(&demo_sntp_client);
#else
    status = nx_sntp_client_run_broadcast(&demo_sntp_client);
#endif  /* USE_UNICAST */

    if (status != NX_SUCCESS)
    {
        return;
    }

    spin = NX_TRUE;

    /* Now check periodically for time changes. */
    while (spin)
    {
        /* Wait for a server update event. */
        tx_event_flags_get(&sntp_flags, DEMO_SNTP_UPDATE_EVENT,
            TX_OR_CLEAR, &events,
            DEMO_PERIODIC_CHECK_INTERVAL);

        if (events == DEMO_SNTP_UPDATE_EVENT)
        {

            /* Check for valid SNTP server status. */
            status = nx_sntp_client_receiving_updates(&demo_sntp_client,
                &server_status);

            if ((status != NX_SUCCESS) || (server_status == NX_FALSE))
            {

                /* We do not have a valid update. Skip processing any time
                   data. If this happens repeatedly, consider stopping the
                   SNTP Client thread, picking another SNTP server and
                   resuming the SNTP Client thread task (more details about
                   that in the comments at the end of this function).

                   If SNTP Client configurable parameters are too restrictive,
                   such as Max Adjustment, that may also cause valid server
                   updates to be rejected. Configurable parameters, however,
                   cannot be changed at run time.
                 */

                continue;
            }

            /* We have a valid update.  Get the SNTP Client time.  */
            status = nx_sntp_client_get_local_time_extended(&demo_sntp_client,
                &seconds, &fraction,
                NX_NULL, 0);

            /* Convert fraction to microseconds. */
            nx_sntp_client_utility_fraction_to_usecs(fraction, &microseconds);

            milliseconds = ((microseconds + 500) / 1000);

            if (status != NX_SUCCESS)
            {
                printf("Internal error with getting local time 0x%x\n",
                    status);
                error_counter++;
            }
            else
            {
                printf("\nSNTP updated\n");
                printf("Time: %lu.%03lu sec.\r\n", seconds, milliseconds);
            }

            /* Clear all events in our event flag. */
            events = 0;
        }
        else
        {

            /* No SNTP update event.
               In the meantime, if we have an RTC we might want to check
               its notion of time. In this demo, we simulate the passage of
               time on our 'RTC' really just the CPU counter, assuming that
               seconds and milliseconds have previously been set to a base
              (starting) time (as was the SNTP Client before running it)
             */

            seconds += 1;
            milliseconds += 1;

            /* Update our timer. */
            wait += DEMO_PERIODIC_CHECK_INTERVAL;

            /* Check if it is time to display the local 'RTC' time. */
            if (wait >= CHECK_SNTP_UPDATES_TIMEOUT)
            {
                /* It is. Reset the timeout and print local time. */
                wait = 0;

                printf("Time: %lu.%03lu sec.\r\n", seconds, milliseconds);
            }
        }
    }

    /* We can stop the SNTP service if for example we think the SNTP server
       has stopped sending updates.

           To restart the SNTP Client, simply call the
           nx_sntp_client_initialize_unicast or
           nx_sntp_client_initialize_broadcast using another SNTP server IP
           address as input, and resume the SNTP Client by calling
           nx_sntp_client_run_unicast or
           nx_sntp_client_run_broadcast. */
    status = nx_sntp_client_stop(&demo_sntp_client);

    if (status != NX_SUCCESS)
    {
        error_counter++;
    }

    /* When done with the SNTP Client, we delete it */
    status = nx_sntp_client_delete(&demo_sntp_client);

    return;
}


/* This application defined handler for handling an
   impending leap second is not required by
   the SNTP Client. The default handler below only logs the
   event for every time stamp received with the
   leap indicator set.  */

UINT leap_second_handler(NX_SNTP_CLIENT* client_ptr,
    UINT leap_indicator)
{
    NX_PARAMETER_NOT_USED(client_ptr);
    NX_PARAMETER_NOT_USED(leap_indicator);

    /* Handle the leap second handler... */

    return NX_SUCCESS;
}

/* This application defined handler for handling
   a Kiss of Death packet is not required by the SNTP Client.
   A KOD handler should determine if the Client task should continue vs.
   abort sending/receiving time data from its current time server,
   and if aborting if it should remove
   the server from its active server list.

   Note that the KOD list of codes is subject to change. The list
   below is current at the time of this software release. */

UINT kiss_of_death_handler(NX_SNTP_CLIENT* client_ptr, UINT KOD_code)
{

    UINT    remove_server_from_list = NX_FALSE;
    UINT    status = NX_SUCCESS;

    NX_PARAMETER_NOT_USED(client_ptr);

    /* Handle kiss of death by code group. */
    switch (KOD_code)
    {

    case NX_SNTP_KOD_RATE:
    case NX_SNTP_KOD_NOT_INIT:
    case NX_SNTP_KOD_STEP:

        /* Find another server while this one
           is temporarily out of service.  */
        status = NX_SNTP_KOD_SERVER_NOT_AVAILABLE;

        break;

    case NX_SNTP_KOD_AUTH_FAIL:
    case NX_SNTP_KOD_NO_KEY:
    case NX_SNTP_KOD_CRYP_FAIL:

        /* These indicate the server will not
           service client with time updates
           without successful authentication. */


        remove_server_from_list = NX_TRUE;

        break;


    default:

        /* All other codes. Remove server
           before resuming time updates. */

        remove_server_from_list = NX_TRUE;
        break;
    }

    /* Removing the server from the active server list? */
    if (remove_server_from_list)
    {

        /* Let the caller know it has to bail on
           this server before resuming service. */
        status = NX_SNTP_KOD_REMOVE_SERVER;
    }

    return status;
}


/* This application defined handler for notifying SNTP time update event.  */

VOID time_update_callback(NX_SNTP_TIME_MESSAGE* time_update_ptr,
    NX_SNTP_TIME* local_time)
{
    tx_event_flags_set(&sntp_flags, DEMO_SNTP_UPDATE_EVENT, TX_OR);
}