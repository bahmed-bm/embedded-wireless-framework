/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "app_threadx.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdlib.h>

#include "stm32u5xx_hal.h"
#include "stm32u5xx_ll_bus.h"
#include "stm32u5xx_ll_rcc.h"
#include "stm32u5xx_ll_system.h"
#include "stm32u5xx_ll_utils.h"
#include "stm32u5xx_ll_pwr.h"
#include "stm32u5xx_ll_exti.h"
#include "stm32u5xx_ll_gpio.h"
#include "stm32u5xx_ll_usart.h"

#include "ewf_lib.c"
#include "ewf_platform_threadx.c"
#include "ewf_allocator.c"
#include "ewf_allocator_threadx.c"
#include "ewf_tokenizer.c"
#include "ewf_tokenizer_basic.c"
#include "ewf_interface.c"
#include "ewf_interface_stm32_uart.c"
#include "ewf_adapter.c"
#include "ewf_adapter_api_control.c"
#include "ewf_adapter_api_info.c"
#include "ewf_adapter_api_tls_basic.c"
#include "ewf_adapter_api_mqtt_basic.c"
#include "ewf_adapter_api_tcp.c"
#include "ewf_adapter_api_udp.c"
#include "ewf_adapter_api_modem.c"
#include "ewf_adapter_api_modem_general.c"
#include "ewf_adapter_api_modem_network_service.c"
#include "ewf_adapter_api_modem_packet_domain.c"
#include "ewf_adapter_api_modem_sim_utility.c"
#include "ewf_adapter_api_modem_sms.c"

#define USE_QUECTEL_MODEM 1

#if(USE_QUECTEL_MODEM)
#include "ewf_adapter_quectel_bg96.c"
#include "ewf_adapter_quectel_common_tokenizer.c"
#include "ewf_adapter_quectel_common_urc.c"
#include "ewf_adapter_quectel_common_control.c"
#include "ewf_adapter_quectel_common_context.c"
#include "ewf_adapter_quectel_common_info.c"
#include "ewf_adapter_quectel_common_internet.c"
#include "ewf_adapter_quectel_common_ufs.c"
#include "ewf_adapter_quectel_common_mqtt_basic.c"
#include "ewf_adapter_quectel_common_tls_basic.c"
#else
#include "ewf_adapter_renesas_ryz024a.c"
#include "ewf_adapter_renesas_common_tokenizer.c"
#include "ewf_adapter_renesas_common_urc.c"
#include "ewf_adapter_renesas_common_control.c"
//#include "ewf_adapter_renesas_common_context.c"
#include "ewf_adapter_renesas_common_info.c"
#include "ewf_adapter_renesas_common_internet.c"
//#include "ewf_adapter_renesas_common_ufs.c"
#include "ewf_adapter_renesas_common_mqtt_basic.c"
#include "ewf_adapter_renesas_common_tls_basic.c"
#endif


#include "ewf_example.config.h"


#define SAMPLE_DHCP_DISABLE


#include <time.h>

#include "nx_api.h"
#include "nx_tcp.h"
#include "nx_udp.h"
#ifndef SAMPLE_DHCP_DISABLE
#include "nxd_dhcp_client.h"
#endif /* SAMPLE_DHCP_DISABLE */
#include "nxd_dns.h"
#include "nx_secure_tls_api.h"
#include "nxd_sntp_client.h"
#include "ewf_middleware_netxduo.h"

#include "ewf_middleware_netxduo.c"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#if defined ( __GNUC__) && !defined(__clang__)
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_ICACHE_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

ULONG _gettimeofday(void)
{
	return 0;
}
/* Include the sample.  */
extern VOID sample_entry(NX_IP* ip_ptr, NX_PACKET_POOL* pool_ptr, NX_DNS* dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

/* Define the helper thread for running Azure SDK on ThreadX (THREADX IoT Platform).  */
#ifndef SAMPLE_HELPER_STACK_SIZE
#define SAMPLE_HELPER_STACK_SIZE        (4096)
#endif /* SAMPLE_HELPER_STACK_SIZE  */

#ifndef SAMPLE_HELPER_THREAD_PRIORITY
#define SAMPLE_HELPER_THREAD_PRIORITY   (4)
#endif /* SAMPLE_HELPER_THREAD_PRIORITY  */

/* Define user configurable symbols. */
#ifndef SAMPLE_IP_STACK_SIZE
#define SAMPLE_IP_STACK_SIZE            (2048)
#endif /* SAMPLE_IP_STACK_SIZE  */

#ifndef SAMPLE_PACKET_COUNT
#define SAMPLE_PACKET_COUNT             (32)
#endif /* SAMPLE_PACKET_COUNT  */

#ifndef SAMPLE_PACKET_SIZE
#define SAMPLE_PACKET_SIZE              (1536)
#endif /* SAMPLE_PACKET_SIZE  */

#define SAMPLE_POOL_SIZE                ((SAMPLE_PACKET_SIZE + sizeof(NX_PACKET)) * SAMPLE_PACKET_COUNT)

#ifndef SAMPLE_ARP_CACHE_SIZE
#define SAMPLE_ARP_CACHE_SIZE           (512)
#endif /* SAMPLE_ARP_CACHE_SIZE  */

#ifndef SAMPLE_IP_THREAD_PRIORITY
#define SAMPLE_IP_THREAD_PRIORITY       (1)
#endif /* SAMPLE_IP_THREAD_PRIORITY */

#ifndef SAMPLE_SNTP_SYNC_MAX
#define SAMPLE_SNTP_SYNC_MAX             (30)
#endif /* SAMPLE_SNTP_SYNC_MAX */

#ifndef SAMPLE_SNTP_UPDATE_MAX
#define SAMPLE_SNTP_UPDATE_MAX            (30)
#endif /* SAMPLE_SNTP_UPDATE_MAX */

#ifndef SAMPLE_SNTP_UPDATE_INTERVAL
#define SAMPLE_SNTP_UPDATE_INTERVAL       (NX_IP_PERIODIC_RATE / 2)
#endif /* SAMPLE_SNTP_UPDATE_INTERVAL */

/* Default time. GMT: Friday, Jan 1, 2022 12:00:00 AM. Epoch timestamp: 1640995200.  */
#ifndef SAMPLE_SYSTEM_TIME
#define SAMPLE_SYSTEM_TIME                1640995200
#endif /* SAMPLE_SYSTEM_TIME  */

/* Seconds between Unix Epoch (1/1/1970) and NTP Epoch (1/1/1999) */
#define SAMPLE_UNIX_TO_NTP_EPOCH_SECOND   (0x83AA7E80)

ULONG g_ip_address = 0;
ULONG g_network_mask = 0;
ULONG g_gateway_address = 0;
ULONG g_dns_address = 0;

static TX_THREAD        sample_helper_thread;
static NX_PACKET_POOL   pool_0;
static NX_IP            ip_0;
static NX_DNS           dns_0;
#ifndef SAMPLE_DHCP_DISABLE
static NX_DHCP          dhcp_0;
#endif /* SAMPLE_DHCP_DISABLE  */
/* SNTP Instance */
static NX_SNTP_CLIENT   sntp_0;

/* Define the stack/cache for ThreadX.  */
static ULONG sample_ip_stack[SAMPLE_IP_STACK_SIZE / sizeof(ULONG)];
#ifndef SAMPLE_POOL_STACK_USER
static ULONG sample_pool_stack[SAMPLE_POOL_SIZE / sizeof(ULONG)];
static ULONG sample_pool_stack_size = sizeof(sample_pool_stack);
#else
extern ULONG sample_pool_stack[];
extern ULONG sample_pool_stack_size;
#endif
static ULONG sample_arp_cache_area[SAMPLE_ARP_CACHE_SIZE / sizeof(ULONG)];
static ULONG sample_helper_thread_stack[SAMPLE_HELPER_STACK_SIZE / sizeof(ULONG)];

/* Define the prototypes for sample thread.  */
static void sample_helper_thread_entry(ULONG parameter);

#ifndef SAMPLE_DHCP_DISABLE
static void dhcp_wait();
#endif /* SAMPLE_DHCP_DISABLE */

static UINT dns_create();

static ULONG unix_time_base;
static UINT unix_time_get(ULONG *unix_time);
static UINT sntp_time_sync();

static const CHAR* sntp_servers[] =
{
  "0.pool.ntp.org",
  "1.pool.ntp.org",
  "2.pool.ntp.org",
  "3.pool.ntp.org",
};
static UINT sntp_server_index;

void ewf_adapter_quectel_bg96_stmod_power_on()
{
#if(0) //bahmed
    /* Reference: Quectel BG96 Hardware Design V1.4 */
    HAL_GPIO_WritePin(STMD_RESET_GPIO_Port, STMD_RESET_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STMD_PWR_EN_GPIO_Port, STMD_PWR_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(STMD_DTR_GPIO_Port, STMD_DTR_Pin, GPIO_PIN_RESET);

    /* Turn OFF the module, in case of error in previous power cycle
      * Set the STMD_PWR_EN_Pin to at least 650ms */
    HAL_GPIO_WritePin(STMD_PWR_EN_GPIO_Port, STMD_PWR_EN_Pin, GPIO_PIN_SET);
    tx_thread_sleep(70U);
    HAL_GPIO_WritePin(STMD_PWR_EN_GPIO_Port, STMD_PWR_EN_Pin, GPIO_PIN_RESET);
    tx_thread_sleep(100U);

    /* Power ON sequence */
    /* Set STMD_PWR_EN_Pin to 1 (initial state) */
    HAL_GPIO_WritePin(STMD_PWR_EN_GPIO_Port, STMD_PWR_EN_Pin, GPIO_PIN_SET);
    tx_thread_sleep(5);

    /* Set STMD_PWR_EN_Pin to 0 during at least 30ms */
    HAL_GPIO_WritePin(STMD_PWR_EN_GPIO_Port, STMD_PWR_EN_Pin, GPIO_PIN_RESET);
    tx_thread_sleep(4);

    /* Set STMD_PWR_EN_Pin to 1 during at least 500ms */
    HAL_GPIO_WritePin(STMD_PWR_EN_GPIO_Port, STMD_PWR_EN_Pin, GPIO_PIN_SET);
    tx_thread_sleep(65U);

    /* Set STMD_PWR_EN_Pin to 0 */
    HAL_GPIO_WritePin(STMD_PWR_EN_GPIO_Port, STMD_PWR_EN_Pin, GPIO_PIN_RESET);
#endif

    /* Wait for the modem to reach ready state  */
    printf("Waiting for BG96 modem to be ready after power ON\n");
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND * 10);
    printf("Modem Ready!\n");

    /* set DTR to 1 */
    HAL_GPIO_WritePin(STMD_DTR_GPIO_Port, STMD_DTR_Pin, GPIO_PIN_SET);

    /* Initialize UART3 */
    MX_USART3_UART_Init();
}


void thread_sample_entry(ULONG thread_input)
{
    /* Power on the STMOD+ BG96 modem */
    ewf_adapter_quectel_bg96_stmod_power_on();

    ewf_result result;
    UINT status=0;

    ewf_allocator* message_allocator_ptr = NULL;
    ewf_interface* interface_ptr = NULL;
    ewf_adapter* adapter_ptr = NULL;

    ULONG   ip_address = 0;
    ULONG   network_mask = 0;
    ULONG   gateway_address = 0;


    EWF_ALLOCATOR_THREADX_STATIC_DECLARE(message_allocator_ptr, message_allocator,
        EWF_CONFIG_MESSAGE_ALLOCATOR_BLOCK_COUNT,
        EWF_CONFIG_MESSAGE_ALLOCATOR_BLOCK_SIZE);
    EWF_INTERFACE_STM32_UART_STATIC_DECLARE(interface_ptr, stm32_uart_port, &huart3);
    //EWF_ADAPTER_QUECTEL_BG96_STATIC_DECLARE(adapter_ptr, quectel_bg96, message_allocator_ptr, NULL, interface_ptr);
    EWF_ADAPTER_RENESAS_RYZ024A_STATIC_DECLARE(adapter_ptr, renesas_ryz024a, message_allocator_ptr, NULL, interface_ptr);

    /* Start the adapter.  */
    if (ewf_result_failed(result = ewf_adapter_start(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to start the adapter, ewf_result %d.\n", result);
        exit(result);
    }

    // Set the SIM PIN
    if (ewf_result_failed(result = ewf_adapter_modem_sim_pin_enter(adapter_ptr, EWF_CONFIG_SIM_PIN)))
    {
        EWF_LOG_ERROR("Failed to the SIM PIN, ewf_result %d.\n", result);
        exit(result);
    }

    // Set the ME functionality
    if (ewf_result_failed(result = ewf_adapter_modem_functionality_set(adapter_ptr, "1")))
    {
        EWF_LOG_ERROR("Failed to the ME functionality, ewf_result %d.\n", result);
        exit(result);
    }

    /* Run the adapter tests.  */
    if (ewf_result_failed(result = ewf_adapter_info(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to run the adapter test, ewf_result %d.\n", result);
        exit(result);
    }

    EWF_LOG("\nDone!\n");


    // Start the adapter
        if (ewf_result_failed(result = ewf_adapter_start(adapter_ptr)))
        {
            EWF_LOG_ERROR("Failed to start the adapter, ewf_result %d.\n", result);
            exit(result);
        }

        // Set the SIM PIN
        if (ewf_result_failed(result = ewf_adapter_modem_sim_pin_enter(adapter_ptr, EWF_CONFIG_SIM_PIN)))
        {
            EWF_LOG_ERROR("Failed to the SIM PIN, ewf_result %d.\n", result);
            exit(result);
        }

        // Enable full functionality
        if (ewf_result_failed(result = ewf_adapter_modem_functionality_set(adapter_ptr, EWF_ADAPTER_MODEM_FUNCTIONALITY_FULL)))
        {
            EWF_LOG_ERROR("Failed to set the ME functionality, ewf_result %d.\n", result);
            exit(result);
        }

        // Activated the PDP context
        if (ewf_result_failed(result = ewf_adapter_quectel_bg96_context_activate(adapter_ptr, EWF_CONFIG_CONTEXT_ID)))
        {
            EWF_LOG("[WARNING] Failed to activate the PDP context, ewf_result %d.\n", result);
            // continue despite the error, the context may be already active
        }

        if (ewf_result_failed(result = ewf_adapter_get_ipv4_address(adapter_ptr, (uint32_t*)&g_ip_address)))
        {
            EWF_LOG_ERROR("Failed to get the adapter IPv4 address: ewf_result %d.\n", result);
            exit(result);
        }

        if (ewf_result_failed(result = ewf_adapter_get_ipv4_netmask(adapter_ptr, (uint32_t*)&g_network_mask)))
        {
            EWF_LOG_ERROR("Failed to get the adapter IPv4 netmask: ewf_result %d.\n", result);
            exit(result);
        }

        if (ewf_result_failed(result = ewf_adapter_get_ipv4_gateway(adapter_ptr, (uint32_t*)&g_gateway_address)))
        {
            EWF_LOG_ERROR("Failed to get the adapter IPv4 gateway: ewf_result %d.\n", result);
            exit(result);
        }

        if (ewf_result_failed(result = ewf_adapter_get_ipv4_dns(adapter_ptr, (uint32_t*)&g_dns_address)))
        {
            EWF_LOG_ERROR("Failed to get the adapter IPv4 DNS: ewf_result %d.\n", result);
            exit(result);
        }

        /* Initialize the NetX system.  */
        nx_system_initialize();

        /* Create a packet pool.  */
        status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", SAMPLE_PACKET_SIZE,
                                       (UCHAR*)sample_pool_stack, sample_pool_stack_size);

        /* Check for pool creation error.  */
        if (status)
        {
            printf("nx_packet_pool_create fail: %u\r\n", status);
            return;
        }

        /* Create an IP instance.  */
        status = nx_ip_create(&ip_0, "NetX IP Instance 0",
            g_ip_address, g_network_mask,
            &pool_0,
            nx_driver_ewf_adapter,
            (UCHAR*)sample_ip_stack, sizeof(sample_ip_stack),
            SAMPLE_IP_THREAD_PRIORITY);

        /* Check for IP create errors.  */
        if (status)
        {
            printf("nx_ip_create fail: %u\r\n", status);
            return;
        }

        /* Save the adapter pointer in the IP instance */
        ip_0.nx_ip_interface->nx_interface_additional_link_info = adapter_ptr;

        /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
        status = nx_arp_enable(&ip_0, (VOID*)sample_arp_cache_area, sizeof(sample_arp_cache_area));

        /* Check for ARP enable errors.  */
        if (status)
        {
            printf("nx_arp_enable fail: %u\r\n", status);
            return;
        }

        /* Enable ICMP traffic.  */
        status = nx_icmp_enable(&ip_0);

        /* Check for ICMP enable errors.  */
        if (status)
        {
            printf("nx_icmp_enable fail: %u\r\n", status);
            return;
        }

        /* Enable TCP traffic.  */
        status = nx_tcp_enable(&ip_0);

        /* Check for TCP enable errors.  */
        if (status)
        {
            printf("nx_tcp_enable fail: %u\r\n", status);
            return;
        }

        /* Enable UDP traffic.  */
        status = nx_udp_enable(&ip_0);

        /* Check for UDP enable errors.  */
        if (status)
        {
            printf("nx_udp_enable fail: %u\r\n", status);
            return;
        }

        /* Initialize TLS.  */
        nx_secure_tls_initialize();

    #ifndef SAMPLE_DHCP_DISABLE
        dhcp_wait();
    #else
        nx_ip_gateway_address_set(&ip_0, g_gateway_address);
    #endif /* SAMPLE_DHCP_DISABLE  */

        /* Get IP address and gateway address. */
        nx_ip_address_get(&ip_0, &ip_address, &network_mask);
        nx_ip_gateway_address_get(&ip_0, &gateway_address);

        /* Output IP address and gateway address. */
        printf("IP address: %lu.%lu.%lu.%lu\r\n",
               (ip_address >> 24),
               (ip_address >> 16 & 0xFF),
               (ip_address >> 8 & 0xFF),
               (ip_address & 0xFF));
        printf("Mask: %lu.%lu.%lu.%lu\r\n",
               (network_mask >> 24),
               (network_mask >> 16 & 0xFF),
               (network_mask >> 8 & 0xFF),
               (network_mask & 0xFF));
        printf("Gateway: %lu.%lu.%lu.%lu\r\n",
               (gateway_address >> 24),
               (gateway_address >> 16 & 0xFF),
               (gateway_address >> 8 & 0xFF),
               (gateway_address & 0xFF));

        /* Create DNS.  */
        status = dns_create();

        /* Check for DNS create errors.  */
        if (status)
        {
            printf("dns_create fail: %u\r\n", status);
            return;
        }

        /* Sync up time by SNTP at start up. */
        status = sntp_time_sync();

        /* Check status.  */
        if (status)
        {
            printf("SNTP Time Sync failed.\r\n");

    #if 1
            printf("Using the time() function if available.\n");
            unix_time_base = (ULONG)time(NULL);
    #else
            printf("Set Time to default value: SAMPLE_SYSTEM_TIME.\n");
            unix_time_base = SAMPLE_SYSTEM_TIME;
    #endif

        }
        else
        {
            printf("SNTP Time Sync successfully.\r\n");
        }

        ULONG unix_time = 0;

        /* Use time to init the seed.  */
        unix_time_get(&unix_time);

        /* Use time to init the seed. FIXME: use real rand on device.  */
        srand((unsigned int)time(NULL));

        /* Start sample.  */
        sample_entry(&ip_0, &pool_0, &dns_0, unix_time_get);
    /* Stay here forever.  */
    while (1)
    {
        EWF_LOG(".");
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
    }
}

static UINT dns_create()
{

UINT    status;
ULONG   dns_server_address[3];
UINT    dns_server_address_size = 12;

    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
    status = nx_dns_create(&dns_0, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    /* Is the DNS client configured for the host application to create the packet pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

    /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages. */
    status = nx_dns_packet_pool_set(&dns_0, ip_0.nx_ip_default_packet_pool);
    if (status)
    {
        nx_dns_delete(&dns_0);
        return(status);
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */

#ifndef SAMPLE_DHCP_DISABLE
    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(&dhcp_0, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address),
                                           &dns_server_address_size);
#else
    dns_server_address[0] = g_dns_address;
#endif /* SAMPLE_DHCP_DISABLE */

    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_0, dns_server_address[0]);
    if (status)
    {
        nx_dns_delete(&dns_0);
        return(status);
    }

    /* Output DNS Server address.  */
    printf("DNS Server address: %lu.%lu.%lu.%lu\r\n",
           (dns_server_address[0] >> 24),
           (dns_server_address[0] >> 16 & 0xFF),
           (dns_server_address[0] >> 8 & 0xFF),
           (dns_server_address[0] & 0xFF));

    return(NX_SUCCESS);
}

/* Sync up the local time. */
static UINT sntp_time_sync_internal(ULONG sntp_server_address)
{
    UINT status;
    UINT server_status;
    UINT i;

    /* Create the SNTP Client to run in broadcast mode.. */
    status = nx_sntp_client_create(&sntp_0, &ip_0, 0, &pool_0, NX_NULL, NX_NULL,
        NX_NULL /* no random_number_generator callback */);

    /* Check status. */
    if (status == NX_SUCCESS)
    {
        /* Use the IPv4 service to initialize the Client and set the IPv4 SNTP server. */
        status = nx_sntp_client_initialize_unicast(&sntp_0, sntp_server_address);

        /* Check status. */
        if (status != NX_SUCCESS)
        {
            nx_sntp_client_delete(&sntp_0);
            return (status);
        }

        /* Set local time to 0 */
        status = nx_sntp_client_set_local_time(&sntp_0, 0, 0);

        /* Check status. */
        if (status != NX_SUCCESS)
        {
            nx_sntp_client_delete(&sntp_0);
            return (status);
        }

        /* Run Unicast client */
        status = nx_sntp_client_run_unicast(&sntp_0);

        /* Check status. */
        if (status != NX_SUCCESS)
        {
            nx_sntp_client_stop(&sntp_0);
            nx_sntp_client_delete(&sntp_0);
            return (status);
        }

        /* Wait till updates are received */
        for (i = 0U; i < SAMPLE_SNTP_UPDATE_MAX; i++)
        {
            /* First verify we have a valid SNTP service running. */
            status = nx_sntp_client_receiving_updates(&sntp_0, &server_status);

            /* Check status. */
            if ((status == NX_SUCCESS) && (server_status == NX_TRUE))
            {
                /* Server status is good. Now get the Client local time. */
                ULONG sntp_seconds;
                ULONG sntp_fraction;
                ULONG system_time_in_second;

                /* Get the local time. */
                status = nx_sntp_client_get_local_time(&sntp_0, &sntp_seconds, &sntp_fraction, NX_NULL);

                /* Check status. */
                if (status != NX_SUCCESS)
                {
                    continue;
                }

                /* Get the system time in second. */
                system_time_in_second = tx_time_get() / TX_TIMER_TICKS_PER_SECOND;

                /* Convert to Unix epoch and minus the current system time. */
                unix_time_base = (sntp_seconds - (system_time_in_second + SAMPLE_UNIX_TO_NTP_EPOCH_SECOND));

                /* Time sync successfully. */

                /* Stop and delete SNTP. */
                nx_sntp_client_stop(&sntp_0);
                nx_sntp_client_delete(&sntp_0);

                return (NX_SUCCESS);
            }

            /* Sleep.  */
            tx_thread_sleep(SAMPLE_SNTP_UPDATE_INTERVAL);
        }

        /* Time sync failed. - Return not success. */
        status = NX_NOT_SUCCESSFUL;
        /* Stop and delete SNTP. */
        nx_sntp_client_stop(&sntp_0);
        nx_sntp_client_delete(&sntp_0);
    }

    return (status);
}

/* Sync up the local time.  */
static UINT sntp_time_sync(void)
{
    UINT  status;
    ULONG gateway_address;
    ULONG sntp_server_address[3];

    /* Sync time by SNTP server array. */
    for (UINT i = 0U; i < SAMPLE_SNTP_SYNC_MAX; i++)
    {
        printf("SNTP Time Sync...%s\r\n", sntp_servers[sntp_server_index]);

        /* Make sure the network is still valid. */
        while (nx_ip_gateway_address_get(&ip_0, &gateway_address))
        {
            tx_thread_sleep(NX_IP_PERIODIC_RATE);
        }

        /* Look up SNTP Server address. */
        status = nx_dns_host_by_name_get(&dns_0, (UCHAR*)sntp_servers[sntp_server_index], &sntp_server_address[0],
            (5 * NX_IP_PERIODIC_RATE));

        /* Check status. */
        if (status == NX_SUCCESS)
        {
            /* Start SNTP to sync the local time. */
            status = sntp_time_sync_internal(sntp_server_address[0]);

            /* Check status.  */
            if (status == NX_SUCCESS)
            {
                return (NX_SUCCESS);
            }
        }

        /* Switch SNTP server every time.  */
        sntp_server_index = (sntp_server_index + 1) % (sizeof(sntp_servers) / sizeof(sntp_servers[0]));
    }

    return (NX_NOT_SUCCESSFUL);
}

static UINT unix_time_get(ULONG* unix_time)
{
    /* Return number of seconds since Unix Epoch (1/1/1970 00:00:00).  */
    *unix_time = unix_time_base + (tx_time_get() / TX_TIMER_TICKS_PER_SECOND);

    return(NX_SUCCESS);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the System Power */
  SystemPower_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ICACHE_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  MX_ThreadX_Init();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{

  /*
   * Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
   */
  HAL_PWREx_DisableUCPDDeadBattery();
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_EnableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CN2_STMOD_UART_SEL_GPIO_Port, CN2_STMOD_UART_SEL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, STMD_PWR_EN_Pin|STMD_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, STMD_SIM_SELECT1_Pin|STMD_DTR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STMD_SIM_SELECT0_GPIO_Port, STMD_SIM_SELECT0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CN2_STMOD_UART_SEL_Pin */
  GPIO_InitStruct.Pin = CN2_STMOD_UART_SEL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CN2_STMOD_UART_SEL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STMD_PWR_EN_Pin STMD_RESET_Pin */
  GPIO_InitStruct.Pin = STMD_PWR_EN_Pin|STMD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : STMD_RING_Pin */
  GPIO_InitStruct.Pin = STMD_RING_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(STMD_RING_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : STMD_SIM_DATA_Pin */
  GPIO_InitStruct.Pin = STMD_SIM_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(STMD_SIM_DATA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STMD_SIM_SELECT1_Pin STMD_DTR_Pin */
  GPIO_InitStruct.Pin = STMD_SIM_SELECT1_Pin|STMD_DTR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : STMD_SIM_SELECT0_Pin */
  GPIO_InitStruct.Pin = STMD_SIM_SELECT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STMD_SIM_SELECT0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STMD_SIM_DATAG1_Pin STMD_SIM_CLK_Pin */
  GPIO_InitStruct.Pin = STMD_SIM_DATAG1_Pin|STMD_SIM_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI6_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
* @brief  Retargets the C library printf function to the USART.
* @param  None
* @retval None
*/

PUTCHAR_PROTOTYPE
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART1 and Loop until the end of transmission */
    //ITM_SendChar(ch);
	HAL_UART_Transmit(&huart1, &ch,1,1000);
    return ch;
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM3 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM3) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
