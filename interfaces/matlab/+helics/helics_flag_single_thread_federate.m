function v = helics_flag_single_thread_federate()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 33);
  end
  v = vInitialized;
end
