project(org_imbi_master_mctAnalyzer)

mitk_create_plugin(
  EXPORT_DIRECTIVE MCTANALYZER_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt
)
