function v = helics_log_level_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 38);
  end
  v = vInitialized;
end
