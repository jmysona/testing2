BLD_DIR_REL =../../..
include $(BLD_DIR_REL)/config.mk
include $(BLD_DIR)/mcMd/config.mk
include $(BLD_DIR)/inter/config.mk
include $(BLD_DIR)/util/config.mk
include $(SRC_DIR)/mcMd/patterns.mk
include $(SRC_DIR)/mcMd/sources.mk
include $(SRC_DIR)/inter/sources.mk
include $(SRC_DIR)/util/sources.mk
include $(SRC_DIR)/mcMd/tests/neighbor/sources.mk

all: $(mcMd_tests_neighbor_OBJS)

clean:
	rm -f $(mcMd_tests_neighbor_OBJS) 
	rm -f $(mcMd_tests_neighbor_OBJS:.o=.d)
	rm -f $(mcMd_tests_neighbor_OBJS:.o=)

-include $(mcMd_tests_neighbor_OBJS:.o=.d)
-include $(mcMd_OBJS:.o=.d)
-include $(inter_OBJS:.o=.d)
-include $(util_OBJS:.o=.d)

