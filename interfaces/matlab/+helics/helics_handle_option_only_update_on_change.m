function v = helics_handle_option_only_update_on_change()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 80);
  end
  v = vInitialized;
end
