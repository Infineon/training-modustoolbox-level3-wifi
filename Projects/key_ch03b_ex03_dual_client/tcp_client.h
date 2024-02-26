/******************************************************************************
* File Name:   tcp_client.h
*
* Description: This file contains declaration of task related to TCP client
* operation.
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

/*******************************************************************************
* Macros
********************************************************************************/
#define MAKE_IPV4_ADDRESS(a, b, c, d)           ((((uint32_t) d) << 24) | \
                                                 (((uint32_t) c) << 16) | \
                                                 (((uint32_t) b) << 8) |\
                                                 ((uint32_t) a))

/* Wi-Fi Credentials: Modify WIFI_SSID, WIFI_PASSWORD and WIFI_SECURITY_TYPE
 * to match your Wi-Fi network credentials.
 * Note: Maximum length of the Wi-Fi SSID and password is set to
 * CY_WCM_MAX_SSID_LEN and CY_WCM_MAX_PASSPHRASE_LEN as defined in cy_wcm.h file.
 */

#define WIFI_SSID                         "ssid"
#define WIFI_PASSWORD                     "pswd"

/* Security type of the Wi-Fi access point. See 'cy_wcm_security_t' structure
 * in "cy_wcm.h" for more details.
 */
#define WIFI_SECURITY_TYPE                 CY_WCM_SECURITY_WPA3_WPA2_PSK

/* Maximum number of connection retries to the Wi-Fi network. */
#define MAX_WIFI_CONN_RETRIES             (10u)

/* Wi-Fi re-connection time interval in milliseconds */
#define WIFI_CONN_RETRY_INTERVAL_MSEC     (1000)

#define MAKE_IPV4_ADDRESS(a, b, c, d)     ((((uint32_t) d) << 24) | \
                                          (((uint32_t) c) << 16) | \
                                          (((uint32_t) b) << 8) |\
                                          ((uint32_t) a))

/* Change the server IP address to match the TCP server address (IP address
 * of the PC).
 */
#define TCP_SERVER_PORT                   50007

#define SECURE_TCP_SERVER_PORT            50008

#define keyCLIENT_CERTIFICATE_PEM \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDYTCCAkmgAwIBAgIJAJ5yTQtF1tSRMA0GCSqGSIb3DQEBCwUAMEcxCzAJBgNV\n"\
"BAYTAlVTMREwDwYDVQQIDAhLZW50dWNreTESMBAGA1UEBwwJTGV4aW5ndG9uMREw\n"\
"DwYDVQQKDAhJbmZpbmVvbjAeFw0yMTA5MjkxNDE3MDVaFw0zMTA5MjcxNDE3MDVa\n"\
"MEcxCzAJBgNVBAYTAlVTMREwDwYDVQQIDAhLZW50dWNreTESMBAGA1UEBwwJTGV4\n"\
"aW5ndG9uMREwDwYDVQQKDAhJbmZpbmVvbjCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"\
"ADCCAQoCggEBALN+B6rq8MICPtidqT+c315fCBFJSZlYqF0Fh/LwqNsirkKdm9pC\n"\
"Q6ofG6rKh+BBahpdUyqfF6pw2k7Is9CsnWkObDjQLepipwD2YKqeP580y6/nFuBe\n"\
"ZjYe58fnjFGN8M59IC/Ny3N2l4vAUbz9wGdYhAmAMMpg4Ye1CEcfYNXcrWHE4VYE\n"\
"RvRJWUUgD2rc1Ef21jt37DfkNH4fRAN2OKt3BiC+05yZbZV50GqOAfr8N0pQSjxb\n"\
"KUzrRYuYIWFX7k4EHn2MeV54OQCY6Nnxnf4l2VlTOBUgd+mHPSAs7+SMTKIrc07d\n"\
"MMGM11DIK16FjuhhIUGlzPuoSmIzBS/YdxECAwEAAaNQME4wHQYDVR0OBBYEFAaa\n"\
"qOwW9cZnFSiNAtXI7Qt9x72mMB8GA1UdIwQYMBaAFAaaqOwW9cZnFSiNAtXI7Qt9\n"\
"x72mMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAEd2j6RvIGZL8Dt2\n"\
"auIrzedTs0DDlZONLAdadO66MyxYjDXY6BRuIRMiE/VJhz4yDlfx8sCy/NXnyXBJ\n"\
"JQ3OR2x3oklYWutDtE5H8PtaBcKVIaW4/mKboTaqTh/ndwYfcZ59+QRAZEl24L0u\n"\
"hKu+0le24Jrn8TYmxkahwWOBe32kM0F/RabyPKDOZfrKt/WLCHvk7BlTfjqhgGcO\n"\
"y/396INavo35b0rmcBXBWAt0WItyrXNfq25icdZr21vtX5+Uji5YbnUUhgBOSj5d\n"\
"YXrP4BzzgMeB9ws8RPf/auKVMyQjbsSAgcLxS3IE10Zrh/keUL/CT/D7C2VqfDr3\n"\
"oWMMkmw=\n"\
"-----END CERTIFICATE-----\n"

/* Private key of the TCP client. Copy from the TCP client key
 * generated by OpenSSL (See Readme.md on how to create a private key).
 */
#define keyCLIENT_PRIVATE_KEY_PEM \
"-----BEGIN PRIVATE KEY-----\n"\
"MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCzfgeq6vDCAj7Y\n"\
"nak/nN9eXwgRSUmZWKhdBYfy8KjbIq5CnZvaQkOqHxuqyofgQWoaXVMqnxeqcNpO\n"\
"yLPQrJ1pDmw40C3qYqcA9mCqnj+fNMuv5xbgXmY2HufH54xRjfDOfSAvzctzdpeL\n"\
"wFG8/cBnWIQJgDDKYOGHtQhHH2DV3K1hxOFWBEb0SVlFIA9q3NRH9tY7d+w35DR+\n"\
"H0QDdjirdwYgvtOcmW2VedBqjgH6/DdKUEo8WylM60WLmCFhV+5OBB59jHleeDkA\n"\
"mOjZ8Z3+JdlZUzgVIHfphz0gLO/kjEyiK3NO3TDBjNdQyCtehY7oYSFBpcz7qEpi\n"\
"MwUv2HcRAgMBAAECggEAeH727srh7ZxMTGcrI8VQPq5VYCVu/z8BZ9E6vuDIurPB\n"\
"gH8GDbPGcQKJNFLafLXtYN2d41MCNC2moJUkAcaftdM0278F0/9+VasQofmyhFKR\n"\
"Gvlr5hv5SgPdXdoX3PgI3NYrMWxaVJ0ch3kIZnODIt3NZgNkvbfkL5JciEKMY4Ss\n"\
"rl92TAmUiXZcjVEGDW7onCKjvvCbyurwV26enOtQ8mpF6O0QhlIjnppdlDz3wdnq\n"\
"mSIhjSY3Nr9fdz7N5dY0hevyRoR0ExGr1QvmZ97ieKPFTl/RCWencEfIA2JSaPBr\n"\
"XRqV6yD4vbgBpu2qT3EPuPN/V1ajg4mYI5sYtVz/8QKBgQDs+Qukks24Kq4Jaark\n"\
"+tEUcqwSQUm5tFO3sgSaWvcbmV7pTXKUhU4Mq9xET0m3NE6tYci5kP7w7TZsGw9R\n"\
"ROGDuYjBheWcJHifo5mDJaDnd+ixSqzu+r2VBKWkKEe34nWiBielgqKc7nuVDcvx\n"\
"on/BvO6/XDYErPzRLF7fJTZKRwKBgQDB53pAcWEluv8aTcBS6+6qzFvfjHvrXZBg\n"\
"G9+Ca2OLl8QilDSpHIn/9/Dwo4C8pLvBy/eSQZnHsdoBpK4dqMNBkamzxKn9vonb\n"\
"qFEMcljMi45IAEhOH5KSUSMXWwhadY/VYTYVT7cBaPnw7BqvYeICdRfLhjUu3euH\n"\
"GS6L19CH5wKBgQCCuWwj0Fwt4VV6mdENWhOmvQ+RTnTWPdUE++4wHEg+8F98Qh4Y\n"\
"MmV49gGfqUGYEAHrAYtSWttYmvMvtcnAYkgpe1smrq+YcEIehohz4Xke13YE/5Xg\n"\
"i2+z2glqQlI17XZI+C48zpDYE3MXKbI8zRC/FnN6GustryQUW+7GGGgXlwKBgQCf\n"\
"dkXLiXJjHQ1+DEMF7pEwVS+ZAGdIgIGjCEkfcEFFI2JhLZDBIn86yiSTQWM1wCPI\n"\
"lHzdAqX82/51K4ElUwyAd9IfzLQfwPqOcjV1DvSIApzYCaNs2/ol6iP5qRuNiPDo\n"\
"gjGABTZzKmmiAAlSAiVj7/fJoG1MRaTUTZlB2BHeMQKBgCZ7jAAPscZRBZBZNPzQ\n"\
"2Zm6FeVbt0XzXMJ0/1Xx4jrd8bEtXSG/Sq1m+EYIN/2Qnq6PuD8zQkq7cxg9Vfu1\n"\
"PiTkcANpdsosenj2Q2kpCphVOsfYLYeij4jCCXyDrjQCbP6RVr9puEnUYUoYhrip\n"\
"yTO8PE2F5sYBWRPnN4R3MgGh\n"\
"-----END PRIVATE KEY-----\n"

/* TCP server certificate. In this example this is the RootCA
 * certificate so as to verify the TCP server's identity. */
#define keySERVER_ROOTCA_PEM \
"-----BEGIN CERTIFICATE-----\n"\
"MIIDYTCCAkmgAwIBAgIJAKesn3EKnL30MA0GCSqGSIb3DQEBCwUAMEcxCzAJBgNV\n"\
"BAYTAlVTMREwDwYDVQQIDAhLZW50dWNreTESMBAGA1UEBwwJTGV4aW5ndG9uMREw\n"\
"DwYDVQQKDAhJbmZpbmVvbjAeFw0yMTA5MjkxNDM0MDFaFw0zMTA5MjcxNDM0MDFa\n"\
"MEcxCzAJBgNVBAYTAlVTMREwDwYDVQQIDAhLZW50dWNreTESMBAGA1UEBwwJTGV4\n"\
"aW5ndG9uMREwDwYDVQQKDAhJbmZpbmVvbjCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"\
"ADCCAQoCggEBALQ50t6GQze9Uekak0kU9prdEjDf2AfICo8hem8MvTEw74yQMljL\n"\
"puuzwRCxPIUCkJiezUH0VwlT0OSAlPjn19r/hLUDmcKfUTIwM8ei6YaN8hZy38Hc\n"\
"py41f9V2ExUY6fTWGLhKXakEbZRq2b2dPf6dM0PVFkdC8PgAoQ+0fYe+UwPFmDqS\n"\
"++qyOpNo0U/0CMWe3HjNYbeQlZRnJr/XKnmmhNmXBTG+57rN/svfpIN6gv3SMtcg\n"\
"IAAYd1az9dgybR07Sz7q9Ah8TNWy8zDH0x8BVW7psH0ONaK73VtWhU7z61/XpPse\n"\
"GEBzaLdikOO0TmlmQrVVQKpLvO34H90qUMECAwEAAaNQME4wHQYDVR0OBBYEFLas\n"\
"AabpF/zmkBrXdA708JCL2TgdMB8GA1UdIwQYMBaAFLasAabpF/zmkBrXdA708JCL\n"\
"2TgdMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAE7Za3HZcSxbnKtl\n"\
"b3yqRLkDvciy0EfqGk7l6y0MN75wcN27rHMCSKRJ9tRJl1XSlSrPx/y3qD15yF0f\n"\
"YGTJoKNjrRgvuyfKUipjCh8GqK7dGalJ1FstR9v5xiaDLx1bPwaVm4jUCkEdeyUk\n"\
"lsKxYECls2RbyVMMtxSGTzkUMt1S/PWpTk1t/yOv3THPsToFS/clxkMBeqzloP1T\n"\
"egnrv+GkCWYu9Mp6PnfZQR2sWxyUu+BvVwSBiekY6Vez8RUIQ/D2vMB+tRrJfR89\n"\
"awk0docPF/I4nboELq8hP4lxaPNPZGj5syqNxXB267ywsT6YRjcYteUN+h97X0Fm\n"\
"BKicZts=\n"\
"-----END CERTIFICATE-----\n"

/*******************************************************************************
* Function Prototype
********************************************************************************/
void tcp_client_task(void *arg);
void isr_button_press( void *callback_arg, cyhal_gpio_event_t event);
void connect_to_wifi_ap(void *arg);

#endif /* TCP_CLIENT_H_ */
