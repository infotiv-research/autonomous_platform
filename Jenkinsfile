pipeline {
  agent { dockerfile true }
  // Poll SCM for changes since webhooks are not working with jenkins server on raspberry pi
  triggers {
    //Hourly polling
    pollSCM('H */1 * * *')
  }
  stages {
    stage('Building') {
      //  # High_Level_Control_Computer/HOW_TO_EXTEND.md and BLUEPILL document
      steps {
        sh '''
          ./build_docs.sh
        ''' 
      }
    }
    stage('Quality control') {
      steps {
        echo 'linting ...'
      }
    }
    stage('Testing') {
      steps {
        echo 'testing'
      }
    }
  }
}
