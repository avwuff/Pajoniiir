from collections import deque

class LateObservation:
    """Track monitoring findings separately from strict losses."""
    def __init__(self, count, maximum):
        self.baseline=count;self.count=count;self.baseline_max=maximum;self.maximum=maximum
        self.window=deque();self.events=[];self.peak=0;self.findings=[]
    def update(self, count, maximum, elapsed):
        if count<self.count:raise RuntimeError('Output-late counter reset')
        delta=count-self.count;self.count=count;self.maximum=maximum
        if delta:
            self.window.append((elapsed,delta));self.events.append({'elapsed_s':elapsed,'delta':delta,'since_boot_max_us':maximum})
        while self.window and elapsed-self.window[0][0]>60:self.window.popleft()
        self.peak=max(self.peak,sum(delta for _,delta in self.window))
        if self.peak>=3 and 'three_or_more_late_events_in_sample_observed_60s' not in self.findings:
            self.findings.append('three_or_more_late_events_in_sample_observed_60s')
        if maximum>self.baseline_max and maximum>15000 and 'new_since_boot_max_above_15000us' not in self.findings:
            self.findings.append('new_since_boot_max_above_15000us')
    def summary(self):
        return {'delta':self.count-self.baseline,'baseline_since_boot_max_us':self.baseline_max,
                'final_since_boot_max_us':self.maximum,'max_sample_observed_cluster_60s':self.peak,
                'events':self.events,'investigation_triggers':self.findings}
