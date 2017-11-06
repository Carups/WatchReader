/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by ftapplicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <glib.h>
#include <dlog.h>
#include <app_common.h>
#include <sap.h>
#include <sap_file_transfer.h>
#include "ft_receiver.h"
#include "ft_progressbar.h"
#include "utils.h"

char rcv_file_path[100]; // global path of saved file, initialized in void accept_file()

struct priv {
	sap_agent_h agent;
	sap_file_transaction_h socket;
};

static struct priv priv_data = { 0 };

gboolean file_on_progress = 0;

static void _on_send_completed(sap_file_transaction_h file_transaction,
		sap_ft_transfer_e result, const char *file_path, void *user_data) {

	char error_message[100];
	set_progress_bar_value(1);

	dlog_print(DLOG_INFO, TAG, "# transfer completed");

	////////
	// DBG
	dlog_print(DLOG_INFO, TAG, "# file_path = %s", file_path);

	// fucking debug-logging code
	if (user_data) {
		char* foo = (char*) user_data;
		dlog_print(DLOG_INFO, TAG, "# file_path = %c%c%c%c%c%c%c%c", foo[0],
				foo[1], foo[2], foo[3], foo[4], foo[5], foo[6], foo[7]);
	} else {
		dlog_print(DLOG_INFO, TAG, "# user_data is null");
	}
	// /DBG
	///////

	if (priv_data.socket) {
		sap_file_transfer_destroy(file_transaction);
		priv_data.socket = NULL;
	}

	if (result == SAP_FT_TRANSFER_SUCCESS) {
		sprintf(error_message, "Transfer Completed");
		show_message_popup(error_message);

		/// our magic function - begin reading with this file
		reader_start(file_path);
		///
	} else {
		switch (result) {
		case (SAP_FT_TRANSFER_FAIL_CHANNEL_IO): {
			sprintf(error_message, "Channel IO Error.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_FILE_IO): {
			sprintf(error_message, "File IO Error.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_CMD_DROPPED): {
			sprintf(error_message, "Transfer dropped/");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_PEER_UNRESPONSIVE): {
			sprintf(error_message, "Peer Un Responsive.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_PEER_CONN_LOST): {
			sprintf(error_message, "Connection Lost.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_PEER_CANCELLED): {
			sprintf(error_message, "Peer Cancelled.");
			break;
		}

		case (SAP_FT_TRANSFER_FAIL_SPACE_NOT_AVAILABLE): {
			sprintf(error_message, "No Space.");
			break;
		}

		default:
			sprintf(error_message, "Unknown Error");
		}

		show_message_popup(error_message);
	}
	hide_progress_bar();
	file_on_progress = 0;
}

static void _on_sending_file_in_progress(
		sap_file_transaction_h file_transaction,
		unsigned short int percentage_progress, void *user_data) {
	dlog_print(DLOG_INFO, TAG, "# progress %d", percentage_progress);
	set_progress_bar_value((float) percentage_progress / 100);
	stop_reading();
}

static void __set_file_transfer_cb(sap_file_transaction_h file_socket) {
	dlog_print(DLOG_INFO, TAG, "# set callbacks");
	sap_file_transfer_set_progress_cb(file_socket, _on_sending_file_in_progress,
			NULL);

	sap_file_transfer_set_done_cb(file_socket, _on_send_completed, NULL);
}

void accept_file() {
	int ret;
	// char rcv_file_path[100];
	char *data_path = NULL;

	data_path = app_get_data_path();
	sprintf(rcv_file_path, "%sfile.txt", data_path); // generate filename here
	free(data_path);

	ret = sap_file_transfer_receive(priv_data.socket, rcv_file_path);

	file_on_progress = 1;

	show_progress_bar();
}

void reject_file() {
	int ret;
	ret = sap_file_transfer_reject(priv_data.socket);

	file_on_progress = 0;
}

static void _on_send_file(sap_peer_agent_h peer_agent_h,
		sap_file_transaction_h socket, const char *file_path, void *user_data) {
	file_on_progress = 1;
	priv_data.socket = socket;
	dlog_print(DLOG_INFO, TAG, "# incoming file request.");
	__set_file_transfer_cb(priv_data.socket);
	show_file_req_popup();
}

void conn_terminated(sap_peer_agent_h peer_agent, sap_socket_h socket,
		sap_service_connection_terminated_reason_e result, void *user_data) {
	dlog_print(DLOG_INFO, TAG, "connection terminated");
}

static void on_conn_req(sap_peer_agent_h peer_agent, sap_socket_h socket,
		sap_service_connection_result_e result, void *user_data) {
	sap_peer_agent_accept_service_connection(peer_agent);
	sap_peer_agent_set_service_connection_terminated_cb(peer_agent,
			conn_terminated, NULL);
}

static void on_agent_initialized(sap_agent_h agent,
		sap_agent_initialized_result_e result, void *user_data) {
	switch (result) {
	case SAP_AGENT_INITIALIZED_RESULT_SUCCESS:

		dlog_print(DLOG_DEBUG, TAG, "agent is initialized");

		priv_data.agent = agent;

		sap_file_transfer_set_incoming_file_cb(agent, _on_send_file, NULL);
		sap_agent_set_service_connection_requested_cb(agent, on_conn_req, NULL);

		break;

	case SAP_AGENT_INITIALIZED_RESULT_DUPLICATED:
		dlog_print(DLOG_ERROR, TAG, "duplicate registration");

		break;

	case SAP_AGENT_INITIALIZED_RESULT_INVALID_ARGUMENTS:
		dlog_print(DLOG_ERROR, TAG, "invalid arguments");

		break;

	case SAP_AGENT_INITIALIZED_RESULT_INTERNAL_ERROR:
		dlog_print(DLOG_ERROR, TAG, "internal sap error");

		break;

	default:
		dlog_print(DLOG_ERROR, TAG, "unknown status (%d)", result);

		break;
	}
}

static void _on_device_status_changed(sap_device_status_e status,
		sap_transport_type_e transport_type, void *user_data) {
	dlog_print(DLOG_DEBUG, TAG, "%s, status :%d", __func__, status);

	switch (transport_type) {
	case SAP_TRANSPORT_TYPE_BT:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): bt", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_BLE:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): ble", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_TCP:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): tcp/ip",
				transport_type);
		break;

	case SAP_TRANSPORT_TYPE_USB:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): usb", transport_type);
		break;

	case SAP_TRANSPORT_TYPE_MOBILE:
		dlog_print(DLOG_DEBUG, TAG, "transport_type (%d): mobile",
				transport_type);
		break;

	default:
		dlog_print(DLOG_ERROR, TAG, "unknown transport_type (%d)",
				transport_type);
		break;
	}

	switch (status) {
	case SAP_DEVICE_STATUS_DETACHED:
		dlog_print(DLOG_DEBUG, TAG, "device is not connected.");

		show_message_popup("Device Disconnected.");

		break;

	case SAP_DEVICE_STATUS_ATTACHED:
		dlog_print(DLOG_DEBUG, TAG, "Attached calling find peer now");

		show_message_popup("Device Connected.");

		break;

	default:
		dlog_print(DLOG_ERROR, TAG, "unknown status (%d)", status);
		break;
	}
}

void cancel_file() {
	sap_file_transfer_cancel(priv_data.socket);
	hide_progress_bar();
}

gboolean agent_initialize() {
	int result = 0;

	do {
		result = sap_agent_initialize(priv_data.agent, "/sample/filetransfer",
				SAP_AGENT_ROLE_PROVIDER, on_agent_initialized, NULL);

		dlog_print(DLOG_DEBUG, TAG,
				"SAP >>> getRegisteredServiceAgent() >>> %d", result);
	} while (result != SAP_RESULT_SUCCESS);

	return TRUE;
}

gboolean initialize_sap(void) {
	sap_agent_h agent = NULL;

	sap_agent_create(&agent);

	priv_data.agent = agent;

	agent_initialize();

	sap_set_device_status_changed_cb(_on_device_status_changed, NULL);

	return TRUE;
}
