# Add all the warnings to the files
if( COMPILER_SUPPORTS_WARNINGS )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARN_FLAGS_CXX}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARN_FLAGS_C}")
endif()

add_definitions( -DEESCHEMA )

if( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    if( MSYS )
        # For some reason the big file sim/ngspice_models.cpp creates an issue when
        # compiled in debug mode on msys2
        # (Probably some out of memory issue that crashes Eeschema at start)
        # So compile it in release mode that generates much smaller binaries
        set_source_files_properties( sim/ngspice_models.cpp PROPERTIES COMPILE_FLAGS "-O2" )
    endif()
endif()

if( KICAD_SPICE )
    set( INC_AFTER ${INC_AFTER} ${NGSPICE_INCLUDE_DIR} )

    # Find out the exact libngspice file name
    get_filename_component( NGSPICE_DLL_ABSPATH "${NGSPICE_DLL}" ABSOLUTE )
    get_filename_component( NGSPICE_DLL_FILE "${NGSPICE_DLL_ABSPATH}" NAME )

    set_property( SOURCE sim/ngspice.cpp
        APPEND PROPERTY COMPILE_DEFINITIONS
        NGSPICE_DLL_FILE="${NGSPICE_DLL_FILE}"
        )
endif()

include_directories( BEFORE ${INC_BEFORE} )
include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/common/dialogs
    ${CMAKE_SOURCE_DIR}/libs/sexpr/include
    ${INC_AFTER}
    ./dialogs
    ./libview
    ./symbol_editor
    ./tools
    ./widgets
    )

set( EESCHEMA_SCH_PLUGINS_ALTIUM
    sch_plugins/altium/altium_parser_sch.cpp
    sch_plugins/altium/sch_altium_plugin.cpp
    )


set( EESCHEMA_SCH_PLUGINS_CADSTAR
    sch_plugins/cadstar/cadstar_sch_archive_loader.cpp
    sch_plugins/cadstar/cadstar_sch_archive_parser.cpp
    sch_plugins/cadstar/cadstar_sch_archive_plugin.cpp
    )

set( EESCHEMA_DLGS
    dialogs/dialog_annotate.cpp
    dialogs/dialog_annotate_base.cpp
    dialogs/dialog_bom.cpp
    dialogs/dialog_bom_base.cpp
    dialogs/dialog_change_symbols.cpp
    dialogs/dialog_change_symbols_base.cpp
    dialogs/dialog_choose_symbol.cpp
    dialogs/dialog_edit_symbols_libid.cpp
    dialogs/dialog_edit_symbols_libid_base.cpp
    dialogs/dialog_eeschema_page_settings.cpp
    dialogs/dialog_erc.cpp
    dialogs/dialog_erc_base.cpp
    dialogs/dialog_field_properties.cpp
    dialogs/dialog_field_properties_base.cpp
    dialogs/dialog_global_edit_text_and_graphics.cpp
    dialogs/dialog_global_edit_text_and_graphics_base.cpp
    dialogs/dialog_global_sym_lib_table_config.cpp
    dialogs/dialog_ibis_parser_reporter.cpp
    dialogs/dialog_ibis_parser_reporter_base.cpp
    dialogs/dialog_image_properties.cpp
    dialogs/dialog_image_properties_base.cpp
    dialogs/dialog_junction_props.cpp
    dialogs/dialog_junction_props_base.cpp
    dialogs/dialog_label_properties.cpp
    dialogs/dialog_label_properties_base.cpp
    dialogs/dialog_lib_edit_pin_table.cpp
    dialogs/dialog_lib_edit_pin_table_base.cpp
    dialogs/dialog_lib_new_symbol.cpp
    dialogs/dialog_lib_new_symbol_base.cpp
    dialogs/dialog_lib_shape_properties.cpp
    dialogs/dialog_lib_shape_properties_base.cpp
    dialogs/dialog_lib_symbol_properties.cpp
    dialogs/dialog_lib_symbol_properties_base.cpp
    dialogs/dialog_lib_text_properties.cpp
    dialogs/dialog_lib_text_properties_base.cpp
    dialogs/dialog_lib_textbox_properties.cpp
    dialogs/dialog_lib_textbox_properties_base.cpp
    dialogs/dialog_line_properties.cpp
    dialogs/dialog_line_properties_base.cpp
    dialogs/dialog_migrate_buses.cpp
    dialogs/dialog_migrate_buses_base.cpp
        dialogs/dialog_export_netlist.cpp
        dialogs/dialog_export_netlist_base.cpp
    dialogs/dialog_pin_properties.cpp
    dialogs/dialog_pin_properties_base.cpp
    dialogs/dialog_plot_schematic.cpp
    dialogs/dialog_plot_schematic_base.cpp
    dialogs/dialog_print_using_printer.cpp
    dialogs/dialog_print_using_printer_base.cpp
    dialogs/dialog_rescue_each.cpp
    dialogs/dialog_rescue_each_base.cpp
    dialogs/dialog_sch_import_settings.cpp
    dialogs/dialog_sch_import_settings_base.cpp
    dialogs/dialog_schematic_find.cpp
    dialogs/dialog_schematic_find_base.cpp
    dialogs/dialog_schematic_setup.cpp
    dialogs/dialog_shape_properties.cpp
    dialogs/dialog_shape_properties_base.cpp
    dialogs/dialog_sheet_pin_properties.cpp
    dialogs/dialog_sheet_pin_properties_base.cpp
    dialogs/dialog_sheet_properties.cpp
    dialogs/dialog_sheet_properties_base.cpp
    dialogs/dialog_symbol_fields_table.cpp
    dialogs/dialog_symbol_fields_table_base.cpp
    dialogs/dialog_symbol_properties.cpp
    dialogs/dialog_symbol_properties_base.cpp
    dialogs/dialog_symbol_remap.cpp
    dialogs/dialog_symbol_remap_base.cpp
    dialogs/dialog_text_properties.cpp
    dialogs/dialog_text_properties_base.cpp
    dialogs/dialog_update_from_pcb.cpp
    dialogs/dialog_update_from_pcb_base.cpp
    dialogs/dialog_update_symbol_fields.cpp
    dialogs/dialog_update_symbol_fields_base.cpp
    dialogs/dialog_wire_bus_properties.cpp
    dialogs/dialog_wire_bus_properties_base.cpp
    dialogs/panel_eeschema_color_settings.cpp
    dialogs/panel_eeschema_display_options.cpp
    dialogs/panel_eeschema_display_options_base.cpp
    dialogs/panel_eeschema_annotation_options.cpp
    dialogs/panel_eeschema_annotation_options_base.cpp
    dialogs/panel_eeschema_editing_options.cpp
    dialogs/panel_eeschema_editing_options_base.cpp
    dialogs/panel_setup_buses.cpp
    dialogs/panel_setup_buses_base.cpp
    dialogs/panel_setup_formatting.cpp
    dialogs/panel_setup_formatting_base.cpp
    dialogs/panel_setup_pinmap.cpp
    dialogs/panel_setup_pinmap_base.cpp
    dialogs/panel_sym_color_settings.cpp
    dialogs/panel_sym_color_settings_base.cpp
    dialogs/panel_sym_display_options.cpp
    dialogs/panel_sym_editing_options.cpp
    dialogs/panel_sym_editing_options_base.cpp
    dialogs/panel_sym_lib_table.cpp
    dialogs/panel_sym_lib_table_base.cpp
    dialogs/panel_template_fieldnames.cpp
    dialogs/panel_template_fieldnames_base.cpp
    )

set( EESCHEMA_WIDGETS
    widgets/hierarchy_pane.cpp
    widgets/pin_shape_combobox.cpp
    widgets/pin_type_combobox.cpp
    widgets/symbol_preview_widget.cpp
    widgets/symbol_tree_pane.cpp
    )


set ( EESCHEMA_LIBEDIT_SRCS
    symbol_editor/lib_logger.cpp
    symbol_editor/lib_symbol_library_manager.cpp
    symbol_editor/menubar_symbol_editor.cpp
    symbol_editor/symbol_edit_frame.cpp
    symbol_editor/symbol_editor.cpp
    symbol_editor/symbol_editor_import_export.cpp
    symbol_editor/symbol_editor_plotter.cpp
    symbol_editor/symbol_editor_settings.cpp
    symbol_editor/symbol_editor_undo_redo.cpp
    symbol_editor/toolbars_symbol_editor.cpp

)
set( EESCHEMA_SRCS
    ${EESCHEMA_DLGS}
    ${EESCHEMA_LIBEDIT_SRCS}
    ${EESCHEMA_SCH_PLUGINS_ALTIUM}
    ${EESCHEMA_SCH_PLUGINS_CADSTAR}
    ${EESCHEMA_WIDGETS}
    annotate.cpp
    autoplace_fields.cpp
    bom_plugins.cpp
    bus-wire-junction.cpp
    cmp_library_lexer.cpp
    component_references_lister.cpp
    connection_graph.cpp
    cross-probing.cpp
    ee_collectors.cpp
    eeschema_config.cpp
    eeschema_helpers.cpp
    eeschema_jobs_handler.cpp
    eeschema_settings.cpp
    erc.cpp
    erc_item.cpp
    erc_settings.cpp
    fields_grid_table.cpp
    files-io.cpp
    generate_alias_info.cpp
    picksymbol.cpp
    lib_field.cpp
    lib_item.cpp
    lib_pin.cpp
    lib_shape.cpp
    lib_symbol.cpp
    lib_text.cpp
    lib_textbox.cpp
    libarch.cpp
    menubar.cpp
    pin_numbers.cpp
    pin_type.cpp
    project_rescue.cpp
    project_sch_specific.cpp
    sch_base_frame.cpp
    sch_bitmap.cpp
    sch_bus_entry.cpp
    sch_connection.cpp
    sch_draw_panel.cpp
    sch_edit_frame.cpp
    sch_field.cpp
    sch_io_mgr.cpp
    sch_item.cpp
    sch_junction.cpp
    sch_label.cpp
    sch_line.cpp
    sch_marker.cpp
    sch_no_connect.cpp
    sch_painter.cpp
    sch_pin.cpp
    sch_plotter.cpp
    sch_plugin.cpp
    sch_preview_panel.cpp
    sch_screen.cpp
    sch_shape.cpp
    sch_sheet.cpp
    sch_sheet_path.cpp
    sch_sheet_pin.cpp
    sch_symbol.cpp
    sch_text.cpp
    sch_textbox.cpp
    sch_validators.cpp
    sch_view.cpp
    schematic.cpp
    schematic_settings.cpp
    schematic_undo_redo.cpp
    sheet.cpp
    symbol_async_loader.cpp
    symbol_checker.cpp
    symbol_lib_table.cpp
    symbol_library.cpp
    symbol_library_manager.cpp
    symbol_tree_model_adapter.cpp
    symbol_tree_synchronizing_adapter.cpp
    symbol_viewer_frame.cpp
    toolbars_sch_editor.cpp
    toolbars_symbol_viewer.cpp

    netlist_exporters/netlist_exporter_base.cpp
    netlist_exporters/netlist_exporter_cadstar.cpp
    netlist_exporters/netlist_exporter_kicad.cpp
    netlist_exporters/netlist_exporter_orcadpcb2.cpp
    netlist_exporters/netlist_exporter_spice.cpp
    netlist_exporters/netlist_exporter_spice_model.cpp
    netlist_exporters/netlist_exporter_xml.cpp
    netlist_exporters/netlist_generator.cpp

    sch_plugins/sch_lib_plugin_cache.cpp
    sch_plugins/eagle/sch_eagle_plugin.cpp
    sch_plugins/kicad/sch_sexpr_lib_plugin_cache.cpp
    sch_plugins/kicad/sch_sexpr_plugin_common.cpp
    sch_plugins/kicad/sch_sexpr_parser.cpp
    sch_plugins/kicad/sch_sexpr_plugin.cpp
    sch_plugins/legacy/sch_legacy_lib_plugin_cache.cpp
    sch_plugins/legacy/sch_legacy_plugin.cpp
    sch_plugins/legacy/sch_legacy_plugin_helpers.cpp
    sch_plugins/database/sch_database_plugin.cpp



    # Some simulation features must be built even if libngspice is not linked.
    sim/sim_library.cpp
    sim/sim_library_spice.cpp
    sim/sim_library_kibis.cpp
    sim/sim_lib_mgr.cpp
    sim/sim_model_serializer.cpp
    sim/sim_model.cpp
    sim/sim_model_behavioral.cpp
    sim/sim_model_ideal.cpp
    sim/sim_model_l_mutual.cpp
    sim/sim_model_kibis.cpp
    sim/sim_model_ngspice.cpp
    sim/sim_model_ngspice_data.cpp
    sim/sim_model_r_pot.cpp
    sim/sim_model_raw_spice.cpp
    sim/sim_model_source.cpp
    sim/sim_model_spice.cpp
    sim/sim_model_spice_fallback.cpp
    sim/sim_model_subckt.cpp
    sim/sim_model_switch.cpp
    sim/sim_model_tline.cpp
    sim/sim_model_xspice.cpp
    sim/sim_value.cpp
    sim/spice_settings.cpp
    sim/spice_generator.cpp
    sim/spice_library_parser.cpp
    sim/spice_model_parser.cpp

    sim/kibis/ibis_parser.cpp
    sim/kibis/kibis.cpp

    tools/assign_footprints.cpp
    tools/backannotate.cpp
    tools/ee_actions.cpp
    tools/ee_grid_helper.cpp
    tools/ee_inspection_tool.cpp
    tools/ee_point_editor.cpp
    tools/ee_selection.cpp
    tools/ee_selection_tool.cpp
    tools/sch_drawing_tools.cpp
    tools/sch_edit_tool.cpp
    tools/sch_editor_control.cpp
    tools/sch_editor_conditions.cpp
    tools/sch_line_wire_bus_tool.cpp
    tools/sch_move_tool.cpp
    tools/sch_navigate_tool.cpp
    tools/symbol_editor_control.cpp
    tools/symbol_editor_drawing_tools.cpp
    tools/symbol_editor_edit_tool.cpp
    tools/symbol_editor_move_tool.cpp
    tools/symbol_editor_pin_tool.cpp
    )


set( EESCHEMA_COMMON_SRCS

    )


if( KICAD_SPICE )
    set( EESCHEMA_SRCS
        ${EESCHEMA_SRCS}
        dialogs/dialog_signal_list.cpp
        dialogs/dialog_signal_list_base.cpp
        dialogs/dialog_sim_command.cpp
        dialogs/dialog_sim_command_base.cpp
        dialogs/dialog_sim_model.cpp
        dialogs/dialog_sim_model_base.cpp
        sim/ngspice_circuit_model.cpp
        sim/ngspice.cpp
        sim/sim_panel_base.cpp
        sim/sim_plot_colors.cpp
        sim/sim_plot_frame.cpp
        sim/sim_plot_frame_base.cpp
        sim/sim_plot_panel.cpp
        sim/sim_property.cpp
        sim/sim_workbook.cpp
        sim/spice_simulator.cpp
        sim/spice_value.cpp
        widgets/tuner_slider.cpp
        widgets/tuner_slider_base.cpp
    )
endif()

if( WIN32 )
    if( MINGW )
        # EESCHEMA_RESOURCES variable is set by the macro.
        mingw_resource_compiler( eeschema )
    else()
        set( EESCHEMA_RESOURCES ${CMAKE_SOURCE_DIR}/resources/msw/eeschema.rc )
    endif()
endif()

# Create a C++ compilable string initializer containing markdown text into a *.h file:
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/dialog_bom_help_md.h
    COMMAND ${CMAKE_COMMAND}
        -DinputFile=${CMAKE_CURRENT_SOURCE_DIR}/dialogs/dialog_bom_help.md
        -DoutputFile=${CMAKE_CURRENT_SOURCE_DIR}/dialogs/dialog_bom_help_md.h
        -P ${CMAKE_MODULE_PATH}/BuildSteps/markdown2C.cmake
    DEPENDS ${CMAKE_MODULE_PATH}/BuildSteps/markdown2C.cmake ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/dialog_bom_help.md
    COMMENT "creating ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/dialog_bom_help_md.h
       from ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/dialog_bom_help.md"
    )

set_source_files_properties( dialogs/dialog_bom.cpp
    PROPERTIES
        OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/dialog_bom_help_md.h
    )

# Create a C++ compilable string initializer containing markdown text into a *.h file:
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/sch_text_help_md.h
    COMMAND ${CMAKE_COMMAND}
        -DinputFile=${CMAKE_CURRENT_SOURCE_DIR}/sch_text_help.md
        -DoutputFile=${CMAKE_CURRENT_SOURCE_DIR}/sch_text_help_md.h
        -P ${CMAKE_MODULE_PATH}/BuildSteps/markdown2C.cmake
    DEPENDS ${CMAKE_MODULE_PATH}/BuildSteps/markdown2C.cmake ${CMAKE_CURRENT_SOURCE_DIR}/sch_text_help.md
    COMMENT "creating ${CMAKE_CURRENT_SOURCE_DIR}/sch_text_help_md.h
       from ${CMAKE_CURRENT_SOURCE_DIR}/sch_text_help.md"
    )

set_source_files_properties( sch_text.cpp
    PROPERTIES
        OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sch_text_help_md.h
    )

if( APPLE )
    # setup bundle
    set( EESCHEMA_RESOURCES eeschema.icns eeschema_doc.icns )
    set_source_files_properties( "${CMAKE_CURRENT_SOURCE_DIR}/eeschema.icns" PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
        )
    set_source_files_properties( "${CMAKE_CURRENT_SOURCE_DIR}/eeschema_doc.icns"  PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
        )
    set( MACOSX_BUNDLE_ICON_FILE eeschema.icns )
    set( MACOSX_BUNDLE_GUI_IDENTIFIER org.kicad.kicad )
    set( MACOSX_BUNDLE_NAME eeschema )
endif()


add_executable( eeschema WIN32 MACOSX_BUNDLE
    ${CMAKE_SOURCE_DIR}/common/single_top.cpp
    ${CMAKE_SOURCE_DIR}/common/pgm_base.cpp
    ${EESCHEMA_RESOURCES}
    )
set_source_files_properties( ${CMAKE_SOURCE_DIR}/common/single_top.cpp PROPERTIES
    COMPILE_DEFINITIONS "TOP_FRAME=FRAME_SCH;PGM_DATA_FILE_EXT=\"kicad_sch\";BUILD_KIWAY_DLL"
    )
target_link_libraries( eeschema
    #singletop  # replaces common, giving us restrictive control and link warnings.
    # There's way too much crap coming in from common yet.
    gal
    common
    argparse::argparse
    ${wxWidgets_LIBRARIES}
    )

# the main Eeschema program, in DSO form.
add_library( eeschema_kiface_objects OBJECT
    ${EESCHEMA_SRCS}
    ${EESCHEMA_COMMON_SRCS}
    )

target_include_directories( eeschema_kiface_objects
    PUBLIC
        .
        netlist_exporters
        $<TARGET_PROPERTY:thread-pool,INTERFACE_INCLUDE_DIRECTORIES>
     )

target_link_libraries( eeschema_kiface_objects
    PUBLIC
        common )

# Since we're not using target_link_libraries, we need to explicitly
# declare the dependency
add_dependencies( eeschema_kiface_objects common )

if( KICAD_USE_3DCONNEXION )
    message( STATUS "Including 3Dconnexion SpaceMouse navigation support in eeschema" )
    add_subdirectory( ./navlib )

    target_link_libraries( eeschema_kiface_objects PUBLIC eeschema_navlib)

    add_dependencies( eeschema_kiface_objects eeschema_navlib )
endif()


add_library( eeschema_kiface MODULE
    eeschema.cpp
    )

target_link_libraries( eeschema_kiface
    PRIVATE
        common
        eeschema_kiface_objects
        markdown_lib
        scripting
        sexpr
        ${wxWidgets_LIBRARIES}
    )

if( KICAD_SPICE )
    target_link_libraries( eeschema_kiface
        PRIVATE
            ${NGSPICE_LIBRARY}
        )

    if( MSVC )
        # Allow for MSVC to debug ngspice from the build directory
        add_custom_command( TARGET eeschema_kiface POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NGSPICE_DLL}" "$<TARGET_FILE_DIR:eeschema_kiface>"
            )

        add_custom_command( TARGET eeschema_kiface POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${NGSPICE_CM_DIR}" "$<TARGET_FILE_DIR:eeschema_kiface>/ngspice"
            )
    endif()
endif()

set_target_properties( eeschema_kiface PROPERTIES
    # Decorate OUTPUT_NAME with PREFIX and SUFFIX, creating something like
    # _eeschema.so, _eeschema.dll, or _eeschema.kiface
    OUTPUT_NAME     eeschema
    PREFIX          ${KIFACE_PREFIX}
    SUFFIX          ${KIFACE_SUFFIX}
    )

# The KIFACE is in eeschema.cpp, export it:
set_source_files_properties( eeschema.cpp PROPERTIES
    COMPILE_DEFINITIONS     "BUILD_KIWAY_DLL;COMPILING_DLL"
    )

# if building eeschema, then also build eeschema_kiface if out of date.
add_dependencies( eeschema eeschema_kiface )

if( MAKE_LINK_MAPS )
    # generate link map with cross reference
    set_target_properties( eeschema_kiface PROPERTIES
        LINK_FLAGS "-Wl,-cref,-Map=${KIFACE_PREFIX}eeschema${KIFACE_SUFFIX}.map"
        )
    set_target_properties( eeschema PROPERTIES
        LINK_FLAGS "-Wl,-cref,-Map=eeschema.map"
        )
endif()

# these 2 binaries are a matched set, keep them together:
if( APPLE )
    set_target_properties( eeschema PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${PROJECT_BINARY_DIR}/eeschema/Info.plist
        )

    # puts binaries into the *.app bundle while linking
    set_target_properties( eeschema_kiface PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${OSX_BUNDLE_BUILD_KIFACE_DIR}
        )
    # put individual bundle outside of main bundle as a first step
    # will be pulled into the main bundle when creating main bundle
    install( TARGETS eeschema
        DESTINATION ${KICAD_BIN}
        COMPONENT binary
        )

    install( CODE "
            # override default embedded path settings
            ${OSX_BUNDLE_OVERRIDE_PATHS}

            # do all the work
            include( BundleUtilities )
            fixup_bundle( ${KICAD_BIN}/eeschema.app/Contents/MacOS/eeschema
                \"\"
                \"${PYTHON_FRAMEWORK}\"
                )
            " COMPONENT Runtime
        )

    if( KICAD_SPICE )
        # bundle libngspice and codemodels
        get_filename_component( ABS_LIBNGSPICE ${NGSPICE_LIBRARY} ABSOLUTE )
        get_filename_component( LIBNGSPICE_PATH ${ABS_LIBNGSPICE} DIRECTORY )

        install( DIRECTORY "${LIBNGSPICE_PATH}/"
                DESTINATION "${OSX_BUNDLE_INSTALL_PLUGIN_DIR}/sim"
                FILES_MATCHING PATTERN "*.dylib")
        install( DIRECTORY "${LIBNGSPICE_PATH}/ngspice"
                 DESTINATION "${OSX_BUNDLE_INSTALL_PLUGIN_DIR}/sim" )
    endif()
else()
    if( MSVC )
        target_sources( eeschema_kiface PRIVATE ${CMAKE_SOURCE_DIR}/resources/msw/eeschema-dll.rc )
    endif()

    install( TARGETS eeschema
        DESTINATION ${KICAD_BIN}
        COMPONENT binary
        )
    install( TARGETS eeschema_kiface
        # actual filename subject to change at milestone C)
        # modular-kicad blueprint.
        DESTINATION ${KICAD_KIFACE}
        COMPONENT binary
        )
endif()


if( KICAD_WIN32_INSTALL_PDBS )
    # Get the PDBs to copy over for MSVC
    install(FILES $<TARGET_PDB_FILE:eeschema> DESTINATION ${KICAD_BIN})
    install(FILES $<TARGET_PDB_FILE:eeschema_kiface> DESTINATION ${KICAD_KIFACE})
endif()

# auto-generate cmp_library_lexer.h and cmp_library_keywords.cpp for the component
# library format.
make_lexer(
    eeschema_kiface_objects
    cmp_library.keywords
    cmp_library_lexer.h
    cmp_library_keywords.cpp
    TLIB_T
    )

make_lexer(
    eeschema_kiface_objects
    dialogs/dialog_bom_cfg.keywords
    dialogs/dialog_bom_cfg_lexer.h
    dialogs/dialog_bom_cfg_keywords.cpp
    T_BOMCFG_T
    )

make_lexer(
    eeschema_kiface_objects
    schematic.keywords
    schematic_lexer.h
    schematic_keywords.cpp
    TSCHEMATIC_T
    )

add_subdirectory( plugins )
