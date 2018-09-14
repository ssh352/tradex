/*
* Copyright (c) 2018-2018 the TradeX authors
* All rights reserved.
*
* The project sponsor and lead author is Xu Rendong.
* E-mail: xrd@ustc.edu, QQ: 277195007, WeChat: ustc_xrd
* See the contributors file for names of other contributors.
*
* Commercial use of this code in source and binary forms is
* governed by a LGPL v3 license. You may get a copy from the
* root directory. Or else you should get a specific written
* permission from the project author.
*
* Individual and educational use of this code in source and
* binary forms is governed by a 3-clause BSD license. You may
* get a copy from the root directory. Certainly welcome you
* to contribute code of all sorts.
*
* Be sure to retain the above copyright notice and conditions.
*/

#include "set_field_ape.h"

SetField::SetField() {
	m_syslog = basicx::SysLog_S::GetInstance();

	m_map_set_field_func[120001] = &SetField::SetField_120001_204501;
	m_map_set_field_func[120002] = &SetField::SetField_120002_204502;
	m_map_set_field_func[120003] = &SetField::SetField_120003_204513;
	m_map_set_field_func[120004] = &SetField::SetField_120004_204511;
	m_map_set_field_func[120005] = &SetField::SetField_120005_204545;
	m_map_set_field_func[120006] = &SetField::SetField_120006_204546;
	m_map_set_field_func[130002] = &SetField::SetField_130002_303002;
	m_map_set_field_func[130004] = &SetField::SetField_130004_304101;
	m_map_set_field_func[130005] = &SetField::SetField_130005_304103;
	m_map_set_field_func[130006] = &SetField::SetField_130006_304110; // Ӧʹ�� 304110 ���� 304109 ���� security_type��trans_id ��ֵ
	m_map_set_field_func[130008] = &SetField::SetField_130008_104105;
	m_map_set_field_func[130009] = &SetField::SetField_130009_104106;
}

SetField::~SetField() {
}

bool SetField::SetField_120001_204501( int32_t api_session, Request* request ) { // ����ί���µ� // ���ͬ VIP �ӿ�
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			// FID_JMLX �������� Char // �����
			Fix_SetString( api_session, FID_GDH, request->m_req_json["holder"].asCString() ); // �ɶ��� Char 10
			Fix_SetString( api_session, FID_ZQDM, request->m_req_json["symbol"].asCString() ); // ֤ȯ���� Char 6
			std::string exchange = request->m_req_json["exchange"].asCString();
			Fix_SetString( api_session, FID_JYS, exchange.c_str() ); // ���������� Char 2
			int32_t entr_type = 0; // 1 �޼ۡ�2 �м�
			double price = 0.0;
			int32_t exch_side = request->m_req_json["exch_side"].asInt();
			if( 1 == exch_side || 2 == exch_side ) { // 1 ���롢2 ������29 �깺��30 ��ء�37 ���롢38 �ʳ�
				entr_type = request->m_req_json["entr_type"].asInt();
				if( 1 == entr_type ) { // �޼�
					entr_type = 0; // ������֤����֤��Ϊ 0 �޼�
				}
				else if( 2 == entr_type && "SH" == exchange ) { // �м�
					entr_type = 1; // ������֤Ϊ 1 �м�
				}
				else if( 2 == entr_type && "SZ" == exchange ) { // �м�
					entr_type = 104; // ������֤Ϊ 104 �м�
				}
				else {
					return false;
				}
				price = request->m_req_json["price"].asDouble();
			}
			Fix_SetLong( api_session, FID_DDLX, entr_type ); // �������� Int
			Fix_SetLong( api_session, FID_JYLB, exch_side ); // ������� Int
			Fix_SetDouble( api_session, FID_WTJG, price ); // ί�м۸� Numric 9,3
			Fix_SetLong( api_session, FID_WTSL, request->m_req_json["amount"].asInt() ); // ί������ Int
			// FID_DDYXRQ 1536 N ������Ч����
			// FID_HGRQ 1251 N ���ۻع�����
			// FID_CJBH 522 C12 �ɽ����
			// FID_HYH 1616 C12 �Զ�Լ����
			// FID_DFXW 1935 C6 �Է�ϯλ
			// FID_DFGDH 1936 C10 �Է��ɶ���
			// FID_WTPCH 1017 N ί�����κ�
			// FID_DDJYXZ 1538 N ��������ָ������
			// FID_DDSXXZ 1537 N ����ʱЧ����
			// FID_PDDM 4279 N ��Դ���
			// FID_TDBZ 9134 C �ض���־
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120002_204502( int32_t api_session, Request* request ) { // ����ί�г��� // ������ VIP �ӿڶ��� FID_GDH��FID_JYS
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
//			// FID_JYMM �������� Char 16 // �����
//			Fix_SetString( api_session, FID_GDH, request->m_req_json["holder"].asCString() ); // �ɶ��� Char 10 // �ӿ� VIP ���� // ���ԣ��������������
//			Fix_SetString( api_session, FID_JYS, request->m_req_json["exchange"].asCString() ); // ���������� Char 2 // �ӿ� VIP ���� // ���ԣ��������������
			Fix_SetLong( api_session, FID_WTH, request->m_req_json["order_id"].asInt() ); // ԭί�к� Int
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120003_204513( int32_t api_session, Request* request ) { // ����ί���µ� // ���ͬ VIP �ӿڣ���Ŀǰ FID_WTFS ����֤
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			// FID_JMLX �������� Char // �����
			Fix_SetLong( api_session, FID_COUNT, request->m_req_json["order_numb"].asInt() ); // ί�б��� Int // ������ 60 ��
			Fix_SetString( api_session, FID_FJXX, request->m_req_json["order_list"].asCString() ); // ί����ϸ��Ϣ Char // 30 �ֽڵı������ 6000 �ֽڣ�JYS(2�ֽ�) ZQDM(6�ֽ�) WTLB(1�ֽ�,1��2) WTJG(9�ֽ�,5.3) WTSL(9�ֽ�) DDLX(3�ֽ�)
			// FID_WTPCH 1017 C ������ί�����κ�
			// FID_WTFS 680 N ί�з�ʽ // ���
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120004_204511( int32_t api_session, Request* request ) { // ����ί�г��� // ������ VIP �ӿ����� FID_EN_WTH
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			// FID_JMLX �������� Char // �����
			Fix_SetLong( api_session, FID_WTPCH, request->m_req_json["batch_id"].asInt() ); // ί�����κ� Int
//			Fix_SetString( api_session, FID_EN_WTH, request->m_req_json["batch_ht"].asCString() ); // ����ί�кŷ�Χ Char 6000
			// FID_ZQDM 719 C6 ֤ȯ����
			// FID_WTLB 683 N �������
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120005_204545( int32_t api_session, Request* request ) { // �۹�ͨ����ί�� // ������ VIP �ӿ� FID_DDLX ���� FID_DDJYXZ
	try {
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
			// FID_JYMM �������� Char 16 // �����
			// FID_JMLX �������� Char // �����
			Fix_SetString( api_session, FID_GDH, request->m_req_json["holder"].asCString() ); // �ɶ��� Char 10
			Fix_SetString( api_session, FID_ZQDM, request->m_req_json["symbol"].asCString() ); // ֤ȯ���� Char 6
			std::string exchange = request->m_req_json["exchange"].asCString();
			Fix_SetString( api_session, FID_JYS, exchange.c_str() ); // ���������� Char 2
			int32_t entr_type = 0; // 1 �����޼��̡�2 ��ǿ�޼��̡�3 �������
			double price = 0.0;
			int32_t exch_side = request->m_req_json["exch_side"].asInt();
			if( 1 == exch_side || 2 == exch_side ) { // 1 ���롢2 ����
				entr_type = request->m_req_json["entr_type"].asInt();
				price = request->m_req_json["price"].asDouble();
			}
			Fix_SetLong( api_session, FID_DDJYXZ, entr_type ); // ��������ָ������ Int
			Fix_SetLong( api_session, FID_JYLB, exch_side ); // ������� Int
			Fix_SetDouble( api_session, FID_WTJG, price ); // ί�м۸� Numric 9,3
			Fix_SetLong( api_session, FID_WTSL, request->m_req_json["amount"].asInt() ); // ί������ Int
			// FID_DDLX 1013 N ��������
			// FID_DDYXRQ 1536 N ������Ч����
			// FID_HGRQ 1251 N ���ۻع�����
			// FID_CJBH 522 C12 �ɽ����
			// FID_HYH 1616 C12 �Զ�Լ����
			// FID_DFXW 1935 C6 �Է�ϯλ
			// FID_DFGDH 1936 C10 �Է��ɶ���
			// FID_WTPCH 1017 N ί�����κ�
			// FID_DDSXXZ 1537 N ����ʱЧ����
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_120006_204546( int32_t api_session, Request* request ) { // �۹�ͨί�г��� // ������ VIP �ӿڶ��� FID_GDH��FID_JYS
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
//			// FID_JYMM �������� Char 16 // �����
			Fix_SetString( api_session, FID_GDH, request->m_req_json["holder"].asCString() ); // �ɶ��� Char 10 // �ӿ� VIP ���� // ���ԣ�����ᱨ�����������
			Fix_SetString( api_session, FID_JYS, request->m_req_json["exchange"].asCString() ); // ���������� Char 2 // �ӿ� VIP ���� // ���ԣ�����ᱨ�����������
			Fix_SetLong( api_session, FID_WTH, request->m_req_json["order_id"].asInt() ); // ԭί�к� Int
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130002_303002( int32_t api_session, Request* request ) { // ��ѯ�ͻ��ʽ� // ���ͬ VIP �ӿ�
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
//			// FID_JYMM �������� Char 16 // �����

			// FID_EXFLG 988 N ��չ��־ // 0 ��ѯ������Ϣ��1 ��ѯ��չ��Ϣ
			// FID_BZ 511 C3 ���� // RMB USD HKD
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130004_304101( int32_t api_session, Request* request ) { // ��ѯ�ͻ��ֲ� // ���ͬ VIP �ӿ�
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
//			// FID_JYMM �������� Char 16 // �����

			// FID_GDH �ɶ��� Char 10
			// FID_JYS ���������� Char 2
			// FID_ZQDM ֤ȯ���� Char 6
			// FID_EXFLG ��ѯ��չ��Ϣ��־ Int // 0 ����չ��1 ��ѯ��չ��Ϣ
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130005_304103( int32_t api_session, Request* request ) { // ��ѯ�ͻ�����ί�� // ������ VIP �ӿ����� FID_WTH // �޷�ͨ����д order_id �ɹ�̨����
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
//			// FID_JYMM �������� Char 16 // �����
			Fix_SetLong( api_session, FID_WTH, request->m_req_json["order_id"].asInt() ); // ί�к� Int // ���ԣ�����Ҳ�������
			Fix_SetString( api_session, FID_BROWINDEX, request->m_req_json["brow_index"].asCString() ); // ������ѯ����ֵ Char 128
			Fix_SetString( api_session, FID_CXBZ, "O" ); // ������־ Char 1 // O ί�У�W ������A ����(ί�кͳ���) // �����������ί��
			// FID_GDH �ɶ��� Char 10
			// FID_JYS ���������� Char 2
			// FID_WTLB ί����� Int
			// FID_ZQDM ֤ȯ���� Char 6
			// FID_WTPCH ί�����κ� Int
			// FID_FLAG ��ѯ��־��0 ����ί�У�1 �ɳ���ί�У�Int
			// FID_SORTTYPE 9109 N ����ʽ // 0 ˳��1 ����
            // FID_ROWCOUNT 9110 N ��ѯ��¼��
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130006_304110( int32_t api_session, Request* request ) { // ��ѯ�ͻ����ճɽ� // ������ VIP �ӿ����� FID_WTH // �޷�ͨ����д order_id �ɹ�̨���� // Ҳû�� FID_CXBZ �ֶΣ��ͻ�����������޳ɽ����������
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			// FID_KHH �ͻ��� Char 20 // �����
//			// FID_JYMM �������� Char 16 // �����
			Fix_SetLong( api_session, FID_WTH, request->m_req_json["order_id"].asInt() ); // ί�к� Int // ���ԣ�����Ҳ�������
			Fix_SetString( api_session, FID_BROWINDEX, request->m_req_json["brow_index"].asCString() ); // ������ѯ����ֵ Char 128
			// FID_EN_WTH 705 C255 ί�кŷ�Χ�����ί�кſ����ö��ŷָ� // ���
			// FID_ROWCOUNT 9110 N ��ѯ��¼��
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130008_104105( int32_t api_session, Request* request ) { // ��ѯETF������Ϣ // ���ͬ VIP �ӿ�
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Fix_SetString( api_session, FID_JJDM, request->m_req_json["fund_id_2"].asCString() ); // ������� Char 6
			// FID_JYS ������ Char 2
			// FID_SGDM ETF������� Char 6
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}

bool SetField::SetField_130009_104106( int32_t api_session, Request* request ) { // ��ѯETF�ɷֹ���Ϣ // ���ͬ VIP �ӿ�
	try {
		std::string field_value = "";
		if( NW_MSG_CODE_JSON == request->m_code ) {
			Fix_SetString( api_session, FID_JJDM, request->m_req_json["fund_id_2"].asCString() ); // ������� Char 6
			return true;
		}
	}
	catch( ... ) {
		return false;
	}
	return false;
}
