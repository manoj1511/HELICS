function v = helics_handle_option_strict_type_checking()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 77);
  end
  v = vInitialized;
end
