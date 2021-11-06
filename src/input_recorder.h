struct RecordedInput {
   vector<InputFlags> history;
};

struct InputRecorder {

   bool is_recording = false;
   bool is_playing = false;

   RecordedInput recorded_inputs[10];
   int recording_idx = 0;
   int playing_idx = 0;
   int playing_flag_idx = 0;

   void start_recording()
   {
      is_recording = true;
   }

   void record(InputFlags flags)
   {
      recorded_inputs[recording_idx].history.push_back(flags);
   };

   void stop_recording()
   {
      is_recording = false;

      // trim recording (removes pauses at starting and ending)
      // we might want to move this to the play logic, so we can choose if we want to play
      // with or without empty input flags
      auto& history  = recorded_inputs[recording_idx].history;
      // trim beginning
      {
         int i = 0; int p = -1;
         while(i < history.size() && history[i].key_press == 0)
         {
            p = i;
            i++;
         }
         if(p >= 0)
            history.erase(history.begin(), history.begin() + p);
      }
      // trim ending
      {
         int i = history.size() - 1; int p = -1;
         while(i >= 0 && history[i].key_press == 0)
         {
            p = i;
            i--;
         }
         if(p >= 0)
            history.erase(history.begin() + p, history.end());
      }

      recording_idx ++;
   };

   void start_playing(int recording_id)
   {
      if(recording_id == -1)
         playing_idx = recording_idx - 1;
      else
         playing_idx = recording_id;
      is_playing = true;
   }

   InputFlags play()
   {
      auto& record = recorded_inputs[playing_idx];
      if(playing_flag_idx == record.history.size() - 1)
         stop_playing();

      return record.history[playing_flag_idx++];
   };

   void stop_playing()
   {
      is_playing = false;
      playing_flag_idx = 0;
   }

   void save()
   {

   }

} Input_Recorder;