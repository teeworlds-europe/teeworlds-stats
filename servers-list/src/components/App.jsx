import React, { useState, useEffect } from 'react';
import {
  BrowserRouter as Router,
  Switch,
  Route,
  Link
} from "react-router-dom";
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faDiscord } from '@fortawesome/free-brands-svg-icons';
import ServersList from './ServerList.jsx';
import ServerDetails from './ServerDetails.jsx';
import About from './About.jsx';
import { searchServers } from '../utils.js';
import './reset.scss';
import './App.scss';

const App = () => {
  const [servers, setServers] = useState([]);

  useEffect(() => {
    (async () => {
      setServers(await searchServers());
    })();
  }, []);

  return (
    <Router>
      <div>
        <section className="navbar">
          <nav>
            <ul>
              <li><Link to="/">Teeworlds Europe</Link></li>
              <li><Link to="/servers">Servers</Link></li>
              <li><Link to="/about">About</Link></li>
              <li><a href={DISCORD_INVITE}><FontAwesomeIcon icon={faDiscord} size="lg" /></a></li>
            </ul>
          </nav>
        </section>
        <Switch>
          <Route
            path="/servers/:server/:port"
            render={(props) => <ServerDetails {...props} servers={servers} />}
          />
          <Route
            path="/servers"
            render={(props) => <ServersList {...props} servers={servers} />}
          />
          <Route
            path="/about"
            component={About}
          />
          <Route
            render={(props) => <ServersList {...props} servers={servers} />}
          />
        </Switch>
      </div>
    </Router>
  );
};

export default App;
