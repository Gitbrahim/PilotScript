/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 
/* #define CLOUD_CLIENT_PRIVATE_KEY \
 * 	"-----BEGIN RSA PRIVATE KEY-----\n" \
 * 	"AWS IoT client private key\n"\
 * 	"-----END RSA PRIVATE KEY-----\n"
 * 
 * #define CLOUD_CLIENT_PUBLIC_CERTIFICATE \
 * 	"-----BEGIN CERTIFICATE-----\n" \
 * 	"AWS IoT public certificate\n"\
 * 	"-----END CERTIFICATE-----\n"
 * 
 * #define CLOUD_CA_CERTIFICATE \
 * 	"-----BEGIN CERTIFICATE-----\n" \
 * 	"AWS IoT Root CA certificate\n"\
 * 	"-----END CERTIFICATE-----\n"
	*/

#define CLOUD_CLIENT_PRIVATE_KEY \
	"-----BEGIN RSA PRIVATE KEY-----\n"\
	"MIIEpAIBAAKCAQEArMI/zuLUQNTdw1j0rffJdqYlnj+ekKmwwl8RwPYRTB1MQ+y+\n"\
        "UFafqKIjtVERRUviOlFS51o1BZrbxJAg0TJaKEl8sOfOYn20nE436cur7PuqJFpG\n"\
        "vrD6nzh3w60WpaYL4C7GggKhYGaG3VyWkvYyjc9/HnTxhVUIgRERNLIkoBNG9Mre\n"\
        "h1WazKhYJaM9UhQ//BXjfS6vXIibQqWPnO3x0bYH36BmcQwXx4o7meKHRVpGhEur\n"\
        "K92es3dYxZpEDHiLFl4OmeFfsVFea2Q5lfIzTAFRxPtTPZVZTfhKH2yyOSabIclt\n"\
        "US2Aa7OAlijm+C4b5aX5l45IW4ZlnYUGUzysjwIDAQABAoIBADKar9AtN1lKQS9g\n"\
        "lkODSpsbrfc90Xe0jRdSe8PXEFgIWPeomFK2HQxreiQ5e+KdiMXmXm9wSAm9Q9S/\n"\
        "Fy3QHwKgDAtRy22kPsxuSQiN5W2EhmEhpIllE/ShuMszn2ekTOQ9L16T4Kt5PNYI\n"\
        "sf6YaVSKzESiosCYmOYdlrml2P6eS1d/uSxzO017WMRB7Gj2RIJruiwfvOp8P6l7\n"\
        "mYtLuvOCRf5Abyzqd/Un66M6kNoZeSSiW3daSeiJNWH0QPqINtmhfK0/zZPQ5LnS\n"\
        "IF2MXh2v+eISq8APfZ+ovZZfQoaSPXowR/Z+t1MRFzEqOuIfT/PqygCbURP59hDu\n"\
        "p5GbNLkCgYEA4+dt+R33PIdEbZgh1DamvKZ6+js1+76bD9tT5+KU6EauJAFTiI3B\n"\
        "R6q3ERmzvhGkA7R6pSLKWk+R1OvOSkANGaAPJ4IxIIbBwVVG2rb85enl+rglymEj\n"\
        "y+fZU+m7KP/s4l/eYPHjvLRJkWEfejR2Y4+E3Mcr7Jvk25qov5WiavsCgYEAwg50\n"\
        "nGJxMBS76c+uE0gsIvM9bAbbViVfoIfM8NWFj5nRGf1ki3T8hbuO1o1h0ctieKik\n"\
        "yeWGvCvYolVXR0e3ZYrXG0cMqdrLq0rbt8TnhJdTxJYrDpedw3IC4y24kK4nTgqx\n"\
        "WA2/ELRoZbqcr+4pY+FT71W/Wbe4+3GTJsyAUH0CgYEAlRxzcJ2xiHG8TLRP1egX\n"\
        "+bA3IAejj12rWJmY8UvnP34XK8mOXPbacch5VH/rgT1LgX0RDYCQwrvy30uVQjFc\n"\
        "INr/b0sYV36zwInJ/Qnit47sgLkBSMN7x0qff+6aHP4tgANXnThGV83hoHPaQ3uV\n"\
        "xhg9xrGtxHZPK3uqUwdVKAcCgYA7yrpxaP+ZIfpYa24mcMlwp2PYgjBY1cEmNFGY\n"\
        "LyItAebkzCeLtKsOBaFYSE3Il5MqvEOyFa+zEQt50TPIObevaN0ogXbMaybwC2ux\n"\
        "rtDrELl1zBNONrccfmeXWgIRSifAKxlDmVCys/OqzJn0tzw3kzkKeHAyE6LCjtcZ\n"\
        "UE+E/QKBgQDDSQvP/75Hy9foA7RkyhjziHcAWCvKyeqItum1xUMuwLW0ozfbC/RY\n"\
        "SMys5VUpSneYngyjlnw7AyftDcF3zLbyAGMfU2fPnngrrBBdC43/xVxIH3827nYW\n"\
        "Z4fA9lzq5L2fDzNkXb/HaokFkFO4BpMHQYL0wiM1kAd2idOcTlVgvg==\n"\
	"-----END RSA PRIVATE KEY-----\n"

#define CLOUD_CLIENT_PUBLIC_CERTIFICATE \
        "-----BEGIN PUBLIC KEY-----\n"\
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEArMI/zuLUQNTdw1j0rffJ\n"\
        "dqYlnj+ekKmwwl8RwPYRTB1MQ+y+UFafqKIjtVERRUviOlFS51o1BZrbxJAg0TJa\n"\
        "KEl8sOfOYn20nE436cur7PuqJFpGvrD6nzh3w60WpaYL4C7GggKhYGaG3VyWkvYy\n"\
        "jc9/HnTxhVUIgRERNLIkoBNG9Mreh1WazKhYJaM9UhQ//BXjfS6vXIibQqWPnO3x\n"\
        "0bYH36BmcQwXx4o7meKHRVpGhEurK92es3dYxZpEDHiLFl4OmeFfsVFea2Q5lfIz\n"\
        "TAFRxPtTPZVZTfhKH2yyOSabIcltUS2Aa7OAlijm+C4b5aX5l45IW4ZlnYUGUzys\n"\
        "jwIDAQAB\n"\
        "-----END PUBLIC KEY-----\n"

#define CLOUD_CA_CERTIFICATE \
        "-----BEGIN CERTIFICATE-----\n"\
        "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"\
        "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"\
        "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"\
        "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"\
        "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"\
        "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"\
        "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"\
        "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"\
        "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"\
        "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"\
        "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"\
        "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"\
        "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"\
        "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"\
        "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"\
        "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"\
        "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"\
        "rqXRfboQnoZsG4q5WTP468SQvvG5\n"\
        "-----END CERTIFICATE-----\n"