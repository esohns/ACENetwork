if (UNIX)
 pkg_check_modules (PKG_NETLINK REQUIRED libnl-3.0)
 if (PKG_NETLINK_FOUND)
   set (NETLINK_FOUND TRUE)
 endif (PKG_NETLINK_FOUND)
 
 if (NETLINK_FOUND)
  option (NETLINK_SUPPORT "enable netlink support" ON)
 endif (NETLINK_FOUND)

 if (NETLINK_SUPPORT)
  add_definitions (-DNETLINK_SUPPORT)
 endif (NETLINK_SUPPORT)
endif (UNIX)

