#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=cc
CCC=CC
CXX=CC
FC=f95
AS=as

# Macros
CND_PLATFORM=OracleSolarisStudio-Solaris-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1386528437/net_asynch_netlinksockethandler.o \
	${OBJECTDIR}/_ext/1386528437/net_asynch_tcpsockethandler.o \
	${OBJECTDIR}/_ext/1386528437/net_asynch_udpsockethandler.o \
	${OBJECTDIR}/_ext/1386528437/net_common_tools.o \
	${OBJECTDIR}/_ext/1386528437/net_connection_base.o \
	${OBJECTDIR}/_ext/1386528437/net_connection_manager.o \
	${OBJECTDIR}/_ext/1386528437/net_controller_base.o \
	${OBJECTDIR}/_ext/1386528437/net_ipconnection.o \
	${OBJECTDIR}/_ext/1386528437/net_message.o \
	${OBJECTDIR}/_ext/1386528437/net_message_base.o \
	${OBJECTDIR}/_ext/1386528437/net_messagehandler_base.o \
	${OBJECTDIR}/_ext/1386528437/net_module_headerparser.o \
	${OBJECTDIR}/_ext/1386528437/net_module_messagehandler.o \
	${OBJECTDIR}/_ext/1386528437/net_module_protocolhandler.o \
	${OBJECTDIR}/_ext/1386528437/net_module_runtimestatistic.o \
	${OBJECTDIR}/_ext/1386528437/net_module_sockethandler.o \
	${OBJECTDIR}/_ext/1386528437/net_netlinkconnection.o \
	${OBJECTDIR}/_ext/1386528437/net_netlinksockethandler.o \
	${OBJECTDIR}/_ext/1386528437/net_protocol_layer.o \
	${OBJECTDIR}/_ext/1386528437/net_sessionmessage.o \
	${OBJECTDIR}/_ext/1386528437/net_socketconnection_base.o \
	${OBJECTDIR}/_ext/1386528437/net_sockethandler_base.o \
	${OBJECTDIR}/_ext/1386528437/net_stream.o \
	${OBJECTDIR}/_ext/1386528437/net_stream_asynch_tcpsocket_base.o \
	${OBJECTDIR}/_ext/1386528437/net_stream_asynch_udpsocket_base.o \
	${OBJECTDIR}/_ext/1386528437/net_stream_tcpsocket_base.o \
	${OBJECTDIR}/_ext/1386528437/net_stream_udpsocket_base.o \
	${OBJECTDIR}/_ext/1386528437/net_tcpconnection.o \
	${OBJECTDIR}/_ext/1386528437/net_tcpconnection_base.o \
	${OBJECTDIR}/_ext/1386528437/net_tcpsockethandler.o \
	${OBJECTDIR}/_ext/1386528437/net_transportlayer_base.o \
	${OBJECTDIR}/_ext/1386528437/net_transportlayer_ip_cast.o \
	${OBJECTDIR}/_ext/1386528437/net_transportlayer_netlink.o \
	${OBJECTDIR}/_ext/1386528437/net_transportlayer_tcp.o \
	${OBJECTDIR}/_ext/1386528437/net_transportlayer_udp.o \
	${OBJECTDIR}/_ext/1386528437/net_udpconnection.o \
	${OBJECTDIR}/_ext/1386528437/net_udpsockethandler.o \
	${OBJECTDIR}/_ext/1386528437/stdafx.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -G -KPIC -norunpath -h liblibACENetwork.${CND_DLIB_EXT}

${OBJECTDIR}/_ext/1386528437/net_asynch_netlinksockethandler.o: ../../../src/net_asynch_netlinksockethandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_asynch_netlinksockethandler.o ../../../src/net_asynch_netlinksockethandler.cpp

${OBJECTDIR}/_ext/1386528437/net_asynch_tcpsockethandler.o: ../../../src/net_asynch_tcpsockethandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_asynch_tcpsockethandler.o ../../../src/net_asynch_tcpsockethandler.cpp

${OBJECTDIR}/_ext/1386528437/net_asynch_udpsockethandler.o: ../../../src/net_asynch_udpsockethandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_asynch_udpsockethandler.o ../../../src/net_asynch_udpsockethandler.cpp

${OBJECTDIR}/_ext/1386528437/net_common_tools.o: ../../../src/net_common_tools.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_common_tools.o ../../../src/net_common_tools.cpp

${OBJECTDIR}/_ext/1386528437/net_connection_base.o: ../../../src/net_connection_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_connection_base.o ../../../src/net_connection_base.cpp

${OBJECTDIR}/_ext/1386528437/net_connection_manager.o: ../../../src/net_connection_manager.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_connection_manager.o ../../../src/net_connection_manager.cpp

${OBJECTDIR}/_ext/1386528437/net_controller_base.o: ../../../src/net_controller_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_controller_base.o ../../../src/net_controller_base.cpp

${OBJECTDIR}/_ext/1386528437/net_ipconnection.o: ../../../src/net_ipconnection.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_ipconnection.o ../../../src/net_ipconnection.cpp

${OBJECTDIR}/_ext/1386528437/net_message.o: ../../../src/net_message.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_message.o ../../../src/net_message.cpp

${OBJECTDIR}/_ext/1386528437/net_message_base.o: ../../../src/net_message_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_message_base.o ../../../src/net_message_base.cpp

${OBJECTDIR}/_ext/1386528437/net_messagehandler_base.o: ../../../src/net_messagehandler_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_messagehandler_base.o ../../../src/net_messagehandler_base.cpp

${OBJECTDIR}/_ext/1386528437/net_module_headerparser.o: ../../../src/net_module_headerparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_module_headerparser.o ../../../src/net_module_headerparser.cpp

${OBJECTDIR}/_ext/1386528437/net_module_messagehandler.o: ../../../src/net_module_messagehandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_module_messagehandler.o ../../../src/net_module_messagehandler.cpp

${OBJECTDIR}/_ext/1386528437/net_module_protocolhandler.o: ../../../src/net_module_protocolhandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_module_protocolhandler.o ../../../src/net_module_protocolhandler.cpp

${OBJECTDIR}/_ext/1386528437/net_module_runtimestatistic.o: ../../../src/net_module_runtimestatistic.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_module_runtimestatistic.o ../../../src/net_module_runtimestatistic.cpp

${OBJECTDIR}/_ext/1386528437/net_module_sockethandler.o: ../../../src/net_module_sockethandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_module_sockethandler.o ../../../src/net_module_sockethandler.cpp

${OBJECTDIR}/_ext/1386528437/net_netlinkconnection.o: ../../../src/net_netlinkconnection.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_netlinkconnection.o ../../../src/net_netlinkconnection.cpp

${OBJECTDIR}/_ext/1386528437/net_netlinksockethandler.o: ../../../src/net_netlinksockethandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_netlinksockethandler.o ../../../src/net_netlinksockethandler.cpp

${OBJECTDIR}/_ext/1386528437/net_protocol_layer.o: ../../../src/net_protocol_layer.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_protocol_layer.o ../../../src/net_protocol_layer.cpp

${OBJECTDIR}/_ext/1386528437/net_sessionmessage.o: ../../../src/net_sessionmessage.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_sessionmessage.o ../../../src/net_sessionmessage.cpp

${OBJECTDIR}/_ext/1386528437/net_socketconnection_base.o: ../../../src/net_socketconnection_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_socketconnection_base.o ../../../src/net_socketconnection_base.cpp

${OBJECTDIR}/_ext/1386528437/net_sockethandler_base.o: ../../../src/net_sockethandler_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_sockethandler_base.o ../../../src/net_sockethandler_base.cpp

${OBJECTDIR}/_ext/1386528437/net_stream.o: ../../../src/net_stream.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_stream.o ../../../src/net_stream.cpp

${OBJECTDIR}/_ext/1386528437/net_stream_asynch_tcpsocket_base.o: ../../../src/net_stream_asynch_tcpsocket_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_stream_asynch_tcpsocket_base.o ../../../src/net_stream_asynch_tcpsocket_base.cpp

${OBJECTDIR}/_ext/1386528437/net_stream_asynch_udpsocket_base.o: ../../../src/net_stream_asynch_udpsocket_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_stream_asynch_udpsocket_base.o ../../../src/net_stream_asynch_udpsocket_base.cpp

${OBJECTDIR}/_ext/1386528437/net_stream_tcpsocket_base.o: ../../../src/net_stream_tcpsocket_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_stream_tcpsocket_base.o ../../../src/net_stream_tcpsocket_base.cpp

${OBJECTDIR}/_ext/1386528437/net_stream_udpsocket_base.o: ../../../src/net_stream_udpsocket_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_stream_udpsocket_base.o ../../../src/net_stream_udpsocket_base.cpp

${OBJECTDIR}/_ext/1386528437/net_tcpconnection.o: ../../../src/net_tcpconnection.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_tcpconnection.o ../../../src/net_tcpconnection.cpp

${OBJECTDIR}/_ext/1386528437/net_tcpconnection_base.o: ../../../src/net_tcpconnection_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_tcpconnection_base.o ../../../src/net_tcpconnection_base.cpp

${OBJECTDIR}/_ext/1386528437/net_tcpsockethandler.o: ../../../src/net_tcpsockethandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_tcpsockethandler.o ../../../src/net_tcpsockethandler.cpp

${OBJECTDIR}/_ext/1386528437/net_transportlayer_base.o: ../../../src/net_transportlayer_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_transportlayer_base.o ../../../src/net_transportlayer_base.cpp

${OBJECTDIR}/_ext/1386528437/net_transportlayer_ip_cast.o: ../../../src/net_transportlayer_ip_cast.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_transportlayer_ip_cast.o ../../../src/net_transportlayer_ip_cast.cpp

${OBJECTDIR}/_ext/1386528437/net_transportlayer_netlink.o: ../../../src/net_transportlayer_netlink.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_transportlayer_netlink.o ../../../src/net_transportlayer_netlink.cpp

${OBJECTDIR}/_ext/1386528437/net_transportlayer_tcp.o: ../../../src/net_transportlayer_tcp.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_transportlayer_tcp.o ../../../src/net_transportlayer_tcp.cpp

${OBJECTDIR}/_ext/1386528437/net_transportlayer_udp.o: ../../../src/net_transportlayer_udp.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_transportlayer_udp.o ../../../src/net_transportlayer_udp.cpp

${OBJECTDIR}/_ext/1386528437/net_udpconnection.o: ../../../src/net_udpconnection.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_udpconnection.o ../../../src/net_udpconnection.cpp

${OBJECTDIR}/_ext/1386528437/net_udpsockethandler.o: ../../../src/net_udpsockethandler.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/net_udpsockethandler.o ../../../src/net_udpsockethandler.cpp

${OBJECTDIR}/_ext/1386528437/stdafx.o: ../../../src/stdafx.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1386528437
	$(COMPILE.cc) -fast -g0 -KPIC  -o ${OBJECTDIR}/_ext/1386528437/stdafx.o ../../../src/stdafx.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/liblibACENetwork.${CND_DLIB_EXT}
	${CCADMIN} -clean

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
