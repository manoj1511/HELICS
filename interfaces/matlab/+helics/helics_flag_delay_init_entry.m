function v = helics_flag_delay_init_entry()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 34);
  end
  v = vInitialized;
end
